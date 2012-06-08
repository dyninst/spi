# 1. Iterate all files in condor_traces
# 2. For each file, add record into the priliminary trace object
# 3. From priliminary trace objects
#    3.1. match up connect and accept insert into final trace objects
#    3.2. insert other non-head priliminary trace object into final trace object
#    3.3. insert head priliminary trace object into final head trace object

# Priliminary trace object
# - trace(name, value1, value2, time)
# - host -> pid -> {head or traces} -> trace array
# + output a static svg for better layout

# Final trace object (deduplicate by name/value + sort by time)
# - trace(name, value1, value2, time)
# - trace array
# + output an xml for furture use in javascript

# Replay temporal traces:
# - maintain a cache for status (e.g., user), have the initial value as default


import glob
import pydot
import os
from xml.dom.minidom import parse

#-----------------------------------------------------------
# Data structures
#-----------------------------------------------------------

class Trace:
    def __init__(self, name, value1, value2, time):
        self.name = name
        self.value1 = value1
        self.value2 = value2
        self.time = time

    # Comparison on duplicates
    def __eq__(self, other):
        return (self.name == other.name) and \
               (self.value1 == other.value1) and \
               (self.value2 == self.value2)

    # Comparison on time
    def __lt__(self, other):
        return (self.time < other.time)

    def __str__(self):
        ret_str = "Name: %s, Value1: %s, Value2: %s, Time: %s" % \
                  (self.name, self.value1, self.value2, self.time)
        return ret_str

    def __hash__(self):
        return hash(self.name) ^ hash(self.value1) ^ hash(self.value2)

    name = ''
    value1 = ''
    value2 = ''
    time = 0


class PrelimTraces(dict):
    """Host -> pid -> {head or trace} -> obj"""
    def __getitem__(self, item):
        try:
            return dict.__getitem__(self, item)
        except KeyError:
            value = self[item] = type(self)()
            return value

    def dump(self):
        for host in self:
            print host
            for pid in self[host]:
                print "\t", pid
                for category in self[host][pid]:
                    print "\t\t", category
                    for trace in self[host][pid][category]:
                        print "\t\t\t", trace

#-----------------------------------------------------------
# File operations
#-----------------------------------------------------------
class FileOperation:
    """ Basic file operations """
    def __init__(self, root_path):
        self.path = root_path
    
    def xml_path_generator(self):
        for infile in glob.glob(os.path.join(self.path, '*')):
            for xmlfile in glob.glob(os.path.join(infile,
                                                  '*.xml')):
                yield xmlfile
    path = ''

#-----------------------------------------------------------
# Xml Parser
#-----------------------------------------------------------
class XmlParser:
    """ Xml Parser """
    def __init__(self, file_path, prelim):
        self.file_path = file_path
        self.prelim = prelim

    def parse_xml(self):
        dom = parse(self.file_path)
        # Head
        head_nodes = dom.getElementsByTagName('head')[0].childNodes
        host = self.get_node_value1(head_nodes, 'host')
        pid = self.get_node_value1(head_nodes, 'pid')
        exe_name = self.get_node_value1(head_nodes, 'exe_name')
        user = self.get_node_value2(head_nodes, 'effective_user', 'name')
        exe_name_trace = Trace('exe_name', exe_name, None, 0)
        if 'head' in self.prelim[host][pid]:
            self.prelim[host][pid]['head'].append(exe_name_trace)
        else:
            head_traces = [exe_name_trace]
            self.prelim[host][pid]['head'] = head_traces
        user_trace = Trace('user', user, None, 0)
        self.prelim[host][pid]['head'].append(user_trace)

        # Traces
        if 'traces' not in self.prelim[host][pid]:
            traces_traces = []
            self.prelim[host][pid]['traces'] = traces_traces
        traces_nodes = dom.getElementsByTagName('traces')[0].childNodes
        for trace in traces_nodes:
            trace_type = self.get_attr_value1(trace, 'type')
            trace_time = self.get_attr_value1(trace, 'time')
            if (trace_type == 'fork'):
                fork_trace = Trace('fork', trace.firstChild.nodeValue, None, trace_time)
                self.prelim[host][pid]['traces'].append(fork_trace)
            elif (trace_type == 'connect to'):
                trg_host = self.get_node_value1(trace.childNodes, 'host')
                trg_port = self.get_node_value1(trace.childNodes, 'port')
                connect_trace = Trace('connect', trg_host, trg_port, trace_time)
                self.prelim[host][pid]['traces'].append(connect_trace)
            elif (trace_type == 'accept from'):
                trg_host = self.get_node_value1(trace.childNodes, 'host')
                trg_port = self.get_node_value1(trace.childNodes, 'port')
                accept_trace = Trace('accept', trg_host, trg_port, trace_time)
                self.prelim[host][pid]['traces'].append(accept_trace)


    # <nodes><key></key></nodes>
    def get_node_value1(self, nodes, key):
        for node in nodes:
            if (node.nodeName == key):
                trg_node = node.firstChild
                return trg_node.nodeValue
        return None

    # <nodes><key1><key2></key2></key1></nodes>
    def get_node_value2(self, nodes, key1, key2):
        for node1 in nodes:
            if (node1.nodeName == key1):
                trg_nodes = node1.childNodes
                for node2 in trg_nodes:
                    if node2.nodeName == key2:
                        trg_node = node2.firstChild
                        return trg_node.nodeValue
        return None

    # <nodes><key attr=val></key></nodes>
    def get_attr_value1(self, node, attr):
        if (node.nodeName == 'trace'):
            val = node.getAttribute(attr)
            return val
        return None

    file_path = ''
    prelim = None

#-----------------------------------------------------------
# Information Extractor
#-----------------------------------------------------------
class InfoExtractor:
    """ Mainly to match up connect / accept pair """
    def __init__(self, prelim):
        self.prelim = prelim

    def extract(self):
        for host in self.prelim:
            for pid in self.prelim[host]:
                for trace in self.prelim[host][pid]['traces']:
                    if trace.name == 'fork':
                        from_id = "%s@%s" % (pid, host)
                        to_id = "%s@%s" % (trace.value1, host)
                        new_trace = Trace('fork', from_id, to_id, trace.time)
                        self.final.append(new_trace)
                    elif trace.name == 'connect':
                        from_id = "%s@%s" % (pid, host)
                        to_id = "%s@%s" % (self.get_pid(trace.value1, trace.value2), trace.value1)
                        new_trace = Trace('connect', from_id, to_id, trace.time)
                        self.final.append(new_trace)
        self.final = list(set(self.final))
        self.final.sort()

    # For connect
    def get_pid(self, host, port):
        for pid in self.prelim[host]:
            for trace in self.prelim[host][pid]['traces']:
                if trace.name == 'accept'and trace.value2 == port:
                    return pid

    def temporal_trace_generator(self):
        for trace in self.final:
            yield trace

    def dump(self):
        for trace in self.final:
            print trace

    prelim = None
    final = []

#-----------------------------------------------------------
# Draw graphs
#-----------------------------------------------------------
class GraphArtist:
    """ Produce dot file, and them generate svg graph """
    def __init__(self, prelim, temporal_generator):
        self.prelim = prelim
        self.temporal_trace_list = temporal_generator

    def dump_temporal(self):
        for trace in self.temporal_trace_list:
            print trace

    """ The union of all snapshots """
    def draw_static_graph(self):
        graph = pydot.Dot(graph_type='digraph')
        for host in self.prelim:
            for pid in self.prelim[host]:
                node_id = "%s@%s" % (pid, host)
                node = pydot.Node(node_id, style="filled", fillcolor="green", label=" ", shape="circle")
                graph.add_node(node)
        for trace in self.temporal_trace_list:
            graph.add_edge(pydot.Edge(trace.value1, trace.value2))
        graph.write_svg('output.svg')
        

#-----------------------------------------------------------
# Main function
#-----------------------------------------------------------
def main():
    """ Configurable variables """
    root_path = 'condor_traces';

    """ Parse XML """
    file_op_obj = FileOperation(root_path)
    generator = file_op_obj.xml_path_generator()
    prelim = PrelimTraces()
    for f in generator:
        parser = XmlParser(f, prelim)
        parser.parse_xml()
    # prelim.dump()

    """ Extract information """
    extractor = InfoExtractor(prelim)
    extractor.extract()
    # extractor.dump()

    """ Draw static svg """
    temporal_generator = extractor.temporal_trace_generator()
    artist = GraphArtist(prelim, temporal_generator)
    artist.dump_temporal()
    artist.draw_static_graph()

if __name__ == "__main__":
    main()
