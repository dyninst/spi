"""
Preliminary trace object
------------------------
- trace: (name, value1, value2, time)
- Preliminary: host -> pid -> {'head' or 'traces'} -> array of traces
+ output a static svg for better layout

Final trace object (deduplicate by name/value + sort by time)
-------------------------------------------------------------
- trace: (name, value1, value2, time)
- array of traces
+ output an xml for furture use in javascript OR
+ output a list of static figures
  - maintain a cache for status (e.g., user), have the initial value as default

TODO
----
- Executable letters
- Legend
- User color
- Trace setuid
- Add setuid event
- Trace multiple machines
"""

import logging
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
        if (exe_name.rpartition('/')[2]):
            exe_name = exe_name.rpartition('/')[2]
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
                if trace.firstChild.nodeValue != '4294967295':
                    fork_trace = Trace('fork', trace.firstChild.nodeValue, None, trace_time)
                    self.prelim[host][pid]['traces'].append(fork_trace)
            elif (trace_type == 'clone'):
                if trace.firstChild.nodeValue != '4294967295':
                    clone_trace = Trace('clone', trace.firstChild.nodeValue, None, trace_time)
                    self.prelim[host][pid]['traces'].append(clone_trace)
            elif (trace_type == 'connect to'):
                trg_host = self.get_node_value1(trace.childNodes, 'host')
                if trg_host == '127.0.0.1':
                    trg_host = host
                trg_port = self.get_node_value1(trace.childNodes, 'port')
                connect_trace = Trace('connect', trg_host, trg_port, trace_time)
                self.prelim[host][pid]['traces'].append(connect_trace)
            elif (trace_type == 'accept from'):
                trg_host = self.get_node_value1(trace.childNodes, 'host')
                if trg_host == '127.0.0.1':
                    trg_host = host
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


#-----------------------------------------------------------
# Information Extractor
#-----------------------------------------------------------
class InfoExtractor:
    """ Mainly to match up connect / accept pair """
    def __init__(self, prelim):
        self.prelim = prelim
        self.final = []

    def extract(self):
        for host in self.prelim:
            for pid in self.prelim[host]:
                for trace in self.prelim[host][pid]['traces']:
                    if trace.name == 'fork':
                        from_id = "%s@%s" % (pid, host)
                        to_id = "%s@%s" % (trace.value1, host)
                        new_trace = Trace('fork', from_id, to_id, trace.time)
                        self.final.append(new_trace)
                    elif trace.name == 'clone':
                        from_id = "%s@%s" % (pid, host)
                        to_id = "%s@%s" % (trace.value1, host)
                        new_trace = Trace('clone', from_id, to_id, trace.time)
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
        if host not in self.prelim:
            return

        for pid in self.prelim[host]:
            for trace in self.prelim[host][pid]['traces']:
                if trace.name == 'accept'and trace.value2 == port:
                    return pid

    def temporal_trace_generator(self):
        for trace in self.final:
            yield trace

    def dump(self):
        i = 0
        for trace in self.final:
            print i, trace
            i += 1


#-----------------------------------------------------------
# Draw graphs
#-----------------------------------------------------------
class GraphArtist:
    """ Produce dot file, and them generate svg graph """

    def __init__(self, prelim, temporal_generator):
        # Prelim data structure containing all traces
        self.prelim = prelim

        # The index to assist assigning a letter to each exe_name
        self.exe_abbr_index = 0

        # Mapping from exe_name to a letter
        self.exe_abbr_map = {}
        
        # Temporal trace list, the earlier the first
        self.temporal_trace_list = []

        # Mapping from hostname to a cluster in graphviz
        self.host_cluster_map = {}

        # Mapping from hostname to booleen
        self.cluster_added = {}

        # Mapping from user to color
        self.user_color_map = {'root':'brown1', 'condor':'pink', 'wenbin':'green'}

        for trace in temporal_generator:
            self.temporal_trace_list.append(trace)

    def dump_temporal(self):
        for trace in self.temporal_trace_list:
            logging.info(trace)

    def dump_exe_abbr_map(self):
        for exe_name in self.exe_abbr_map:
            logging.info("%s => %s", exe_name, self.exe_abbr_map[exe_name])

    def get_init_exe_name(self, host, pid):
        """ Get a process's initial exe_name """
        if host not in self.prelim or \
                pid not in self.prelim[host]:
            None
        for trace in self.prelim[host][pid]['head']:
            if (trace.name == "exe_name"):
                return trace.value1
        return None

    def get_user_color(self, host, pid):
        """ TODO(wenbin): track setuid events """
        if host not in self.prelim or \
                pid not in self.prelim[host]:
            return "white"

        for trace in self.prelim[host][pid]['head']:
            if (trace.name == "user"):
                if trace.value1 in self.user_color_map:
                    return self.user_color_map[trace.value1]
        return "white"

        
    def get_exe_abbr(self, host, pid):
        """
        Get executable name's letter abbrebiation, basic steps:
        1. Check the cache (exe_name -> letter). If found, return.
        2. Maintain a counter to index alphabet. Build up the cache.
        """
        exe_name = self.get_init_exe_name(host, pid)
        if exe_name == None:
            return 'Unknown'

        if (exe_name in self.exe_abbr_map):
            return self.exe_abbr_map[exe_name]
        else:
            alphabet = map(chr, range(65, 91))
            self.exe_abbr_map[exe_name] = alphabet[self.exe_abbr_index]
            self.exe_abbr_index += 1
            return self.exe_abbr_map[exe_name]


    def get_cluster(self, node_id_str, graph):
        """
        Get cluster for a given host, basic steps:
        1. Check the cache (hostname -> cluster). If found, return.
        2. Otherwise, creat a cluster and add to parent graph.
        """
        (pid, sep, host) = node_id_str.partition('@')
        if host in self.host_cluster_map:
            return self.host_cluster_map[host]

        cluster_id = "%d" % hash(host)
        cluster = pydot.Cluster(cluster_id,label=host)
        self.host_cluster_map[host] = cluster
        return cluster

    def add_cluster(self, cluster, graph):
        if cluster not in self.cluster_added:
            graph.add_subgraph(cluster)

    def add_node(self):
        pass

    def exist_pid_host(self, pid, host):
        if host not in self.prelim or \
                pid not in self.prelim[host]:
            return False
        return True

    def remove_node(self, node_id_str):
        """ 
        """
        pass

    def rename_node(self, node_id_str):
        """
        We deduplicate such nodes with all these properties:
        1. No traces
        2. Same parent
        """
        pass

    def draw(self, output_dir, static = True):
        """ 
        Draw pictures, use both prelim and final. 
        If static is True, then output $output_dir/static.svn. Otherwise,
        output $output_dir/$i.svn
        """
        len_temporal = len(self.temporal_trace_list)
        logging.info("Length of temporal trace list is %d", len_temporal)
        for i in range(len_temporal):
            graph = pydot.Dot(graph_type='digraph')
            if static == True:
                i = len(self.temporal_trace_list) - 1
                logging.info("***%d", i)

            logging.info("Replay [0, %d) traces", i)
            j = 0
            while True:
                if j > i and j != 0:
                    break
                trace = self.temporal_trace_list[j]
                j += 1
                (from_pid, sep, from_host) = trace.value1.partition('@')
                from_color = self.get_user_color(from_host, from_pid)
                from_exe_abbr = self.get_exe_abbr(from_host, from_pid)
                from_cluster = self.get_cluster(trace.value1, graph)
                (to_pid, sep, to_host) = trace.value2.partition('@')
                to_color = self.get_user_color(to_host, to_pid)
                to_exe_abbr = self.get_exe_abbr(to_host, to_pid)
                to_cluster = self.get_cluster(trace.value2, graph)
                if from_exe_abbr == 'Unknown' or to_exe_abbr == 'Unknown':
                    continue
                if not self.exist_pid_host(from_pid, from_host) or \
                        not self.exist_pid_host(to_pid, to_host):
                    logging.info("Skip %s@%s => %s@%s", from_pid, \
                                     from_host, to_pid, to_host)
                    continue

                border_color = 'black'
                if j > i and static == False:
                    border_color = 'chartreuse'

                from_node = pydot.Node(trace.value1, \
                                      style = "filled", \
                                      color = border_color, \
                                      fillcolor = from_color, \
                                      label = from_exe_abbr, \
                                      shape = "circle")
                from_cluster.add_node(from_node)

                to_node = pydot.Node(trace.value2, \
                                      style="filled", \
                                      color = border_color, \
                                      fillcolor=to_color, \
                                      label=to_exe_abbr, \
                                      shape="circle")
                to_cluster.add_node(to_node)
                logging.info("Drawing %d trace", j-1)

            for host, cluster in self.host_cluster_map.items():
                self.add_cluster(cluster, graph)

            j = 0
            while True:
                if j > i and j != 0:
                    break
                trace = self.temporal_trace_list[j]
                j += 1
                (from_pid, sep, from_host) = trace.value1.partition('@')
                (to_pid, sep, to_host) = trace.value2.partition('@')
                from_exe_abbr = self.get_exe_abbr(from_host, from_pid)
                to_exe_abbr = self.get_exe_abbr(to_host, to_pid)
                if from_exe_abbr == 'Unknown' or to_exe_abbr == 'Unknown':
                    continue

                if self.exist_pid_host(from_pid, from_host) == False or \
                        self.exist_pid_host(to_pid, to_host) == False:
                    logging.info("Skip %s@%s => %s@%s", from_pid, \
                                     from_host, to_pid, to_host)
                    continue
                edge = pydot.Edge(trace.value1, trace.value2)
                # Highlight the last event edge
                if j > i and static == False:
                    edge.set_color('chartreuse')
                if trace.name == "connect":
                    edge.set_style("dotted")
                    edge.set_arrowhead("empty")
                elif trace.name == "clone":
                    edge.set_style("dashed")

                graph.add_edge(edge)

            if static == True:
                output_path = "%s/static.svg" % output_dir
                graph.write_svg(output_path)
                raw_output_path = "%s/static.dot" % output_dir
                graph.write_raw(raw_output_path)
                return
            else:
                output_path = "%s/%d.svg" % (output_dir, i)
                graph.write_svg(output_path)
                raw_output_path = "%s/%d.dot" % (output_dir, i)
                graph.write_raw(raw_output_path)
                event_output_path = "%s/%d.e" % (output_dir, i)
                ef = open(event_output_path, "w")
                ef_str = ""
                if trace.name == "connect":
                    ef_str = "%s connects to %s" % (trace.value1, trace.value2)
                elif trace.name == "clone":
                    ef_str = "%s clones %s" % (trace.value1, trace.value2)
                elif trace.name == "fork":
                    ef_str = "%s forks %s" % (trace.value1, trace.value2)
                ef.write(ef_str)

#-----------------------------------------------------------
# Generate HTML file to display svg
#-----------------------------------------------------------
class HtmlGenerator:
    def __init__(self, output_dir, trace_generator):
        self.output_dir = output_dir
        self.traces = []
        for trace in trace_generator:
            self.traces.append(trace)

    def generate_config(self):
        output_path = "%s/config.js" % self.output_dir
        config_file = open(output_path, "w")
        config_str = "var kNumFrames = %d;" % len(self.traces)
        config_file.write(config_str)
        
#-----------------------------------------------------------
# Main function
#-----------------------------------------------------------
def main():
    #
    # Configurable variables
    #
    logging.basicConfig(level=logging.INFO)
    root_path = 'condor_traces';
    output_dir = 'animation/';

    #
    # Parse XML
    #
    file_op_obj = FileOperation(root_path)
    generator = file_op_obj.xml_path_generator()
    prelim = PrelimTraces()
    for f in generator:
        parser = XmlParser(f, prelim)
        parser.parse_xml()

    prelim.dump()

    #
    # Extract information
    #
    extractor = InfoExtractor(prelim)
    extractor.extract()
    extractor.dump()

    #
    # Draw static svg
    #
    temporal_generator = extractor.temporal_trace_generator()
    artist = GraphArtist(prelim, temporal_generator)

    # FIXME: cannot generate static and animation together!
    # artist.draw(output_dir, True)
    artist.draw(output_dir, False)

    #
    # Generate HTML file to display svg
    #
    temporal_generator = extractor.temporal_trace_generator()
    html_generator = HtmlGenerator(output_dir, temporal_generator)
    html_generator.generate_config()

if __name__ == "__main__":
    main()
