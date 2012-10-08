"""
A wrapper of graphviz dot to generate graphs
"""

import logging
import os
import pydot

from structures import lookup_table
from structures import event

class GraphArtist:
    """
    Attibutes:
      __dot: the instance of pydot.Dot

      __dedup_dict: ppid,init_exe,uid -> event

      __node_map: pid@host -> dot node
      __edge_map: edge[pid@host][pid@host][event_type] -> edge
      __cluster_map: host -> cluster
      __node_edges_map: node -> {edge1, edge2, ...}

      __exe_initial_map: exe (no path) -> initial
      __initial_exe_map: initial -> exe (no path)

      __color_list: available colors
      __color_cur: next available color, making it a circular list
      __uid_color_map: uid -> color
    """
    def __init__(self, table, dedup=False, static=True):
        """
        Args:
        
          event_list: a list of events 
          dedup: whether to remove duplicate leaf nodes
          static: whether to draw a static graph (True) or animation graphs
          (False)

        """

        # Copy event list!
        self.event_list = list(table.event_list)

        # Read-only trace_map
        self.trace_map = table.trace_map

        self.dedup = dedup
        self.static = static

        self.__dedup_dict = dict() 

        self.__cluster_map = dict()
        self.__node_map = dict()
        self.__edge_map = dict()
        self.__node_edges_map = dict()

        self.__exe_initial_map = dict()
        self.__initial_exe_map = dict()

        self.__color_list = ['deeppink1', 'green', 'blue']
        self.__color_cur = 0
        self.__uid_color_map = {'root':'red'}

    def draw(self, output_dir):
        """
        Description:

          Dedup
          - If dedup is True, preprocess event_list
          - Build a lists of processes that requre double circles
        
          Draw static graph
          - Play all events one by one
          - For each event, get cluster (if not exist, create one)
            - fork / clone: add two nodes (init color), one edge (if exist, skip)
            - connect: add two nodes (init color), one edge (if exist, skip)
            - seteuid: get node, change color
            - execve: change letter

          Draw animation graphs
          - Make all nodes and edges invisible
          - Play all events one by one
          - For each event, get cluster
            - fork / clone: add two nodes (init color), one edge (if exist, skip)
            - connect: add two nodes (init color), one edge (if exist, skip)
            - seteuid: get node, change color
            - execve: change letter
            - exit: disappear nodes

          Generate Legend
  
        Args:

          output_dir: output dir for svg images and dot files
        """

        # Dedup first
        dest_dir = "%s/raw" % output_dir
        if self.dedup == True:
            logging.info('- Dedup = True')
            dest_dir = "%s/dedup" % output_dir
            self.__dedup()
        else:
            logging.info('- Dedup = False')

        # Draw static graph
        self.__draw_static()

        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)

        # If we only want to produce static graph, just output it then return
        if self.static == True:
            logging.info('- Static = True')
            self.__dot.write_svg("%s/static.svg" % dest_dir)
            return
        else:
            logging.info('- Static = False')

        # Draw animation graphs
        self.__draw_animation(dest_dir)

        # Generate legend
        self.__generate_legend(dest_dir)


    #
    # Main-logic procedures
    #

    def __dedup(self):
        """
        Description:
          
          Remove "duplicate" child processes that fulfill these requirements:
          1. Same uid
          2. Same init exe name
          3. Same parent
          4. No exe
          5. No setuid event
          6. No fork/clone

          The main mechanism is to keep only one fork/clone event for the same 
          duplicate child processes (implemented in __is_not_dup_event()):
          1. Maintain a dictionary dedup_dict[parent_pid][init_exe][uid]
          2. For each fork/clone event, if not setuid event and 
             fork-then-exe event:
             2.1. If not in dedup_dict, add it
             2.2. else, delete this event from event_list
        """
        self.event_list[:] = [eve for eve in self.event_list \
                                  if self.__is_not_dup_event(eve)]

    def __draw_static(self):
        """
        Description:
          
          Draw a static graph, which retains all final states except for `exit`
        """
        self.__dot = pydot.Dot(graph_type='digraph', graph_name='SecSTAR')
        for eve in self.event_list:
            cluster = self.__get_cluster(eve.host)
            if eve.type == 'connect':
                self.__get_node(eve.pidhost(), cluster)
                self.__get_node(eve.trg_pidhost(), cluster)
                self.__get_edge(eve.pidhost(), eve.trg_pidhost(), eve.type)
            elif eve.type == 'fork' or eve.type == 'clone':
                self.__get_node(eve.pidhost(), cluster)
                self.__get_node(eve.child_pidhost(), cluster)
                self.__get_edge(eve.pidhost(), eve.child_pidhost(), eve.type)
            elif eve.type == 'seteuid':
                self.__get_node(eve.pidhost(), cluster, uid=eve.new_euid)
            elif eve.type == 'execve':
                self.__get_node(eve.pidhost(), cluster, exe=eve.new_exe)
            
    def __draw_animation(self, dest_dir):
        """
        Draw animation diagrams
        1. Make all nodes and edges invisible
        2. Build node edges map to assist handling exit event
        3. Produce a diagram for first k events
        """
        self.__make_all_invisible()
        self.__build_node_edges_map()

        event_count = 0
        for i in range(len(self.event_list)):
            eve_desc = ''
            for j in range(i+1):
                eve = self.event_list[j]
                cluster = self.__get_cluster(eve.host)
                if eve.type == 'connect':
                    node = self.__get_node(eve.pidhost(), cluster)
                    self.__show_node(node)
                    if j == i: self.__highlight_node(node)
                    node = self.__get_node(eve.trg_pidhost(), cluster)
                    self.__show_node(node)
                    if j == i: self.__highlight_node(node)
                    edge = self.__get_edge(eve.pidhost(), eve.trg_pidhost(), eve.type)
                    self.__show_edge(edge, eve.type)
                    if j == i: self.__highlight_edge(edge)
                    eve_desc = "%s<br><font color='red'><b>connects to</b></font><br>%s" % \
                        (eve.pidhost(), eve.trg_pidhost())
        
                elif eve.type == 'fork' or eve.type == 'clone':
                    trace = self.trace_map[eve.host][eve.child]
                    node = self.__get_node(eve.pidhost(), cluster)
                    self.__show_node(node)
                    if j == i: self.__highlight_node(node)
                    node = self.__get_node(eve.child_pidhost(), cluster, \
                                               exe=trace.init_exe, \
                                               uid=trace.init_euid)
                    self.__show_node(node)
                    if j == i: self.__highlight_node(node)
                    edge = self.__get_edge(eve.pidhost(), eve.child_pidhost(), eve.type)
                    self.__show_edge(edge, eve.type)
                    if j == i: self.__highlight_edge(edge)
                    eve_desc = "%s<br><font color='red'><b>forks</b></font><br>%s" % \
                        (eve.pidhost(), eve.child_pidhost())
        
                elif eve.type == 'seteuid':
                    node = self.__get_node(eve.pidhost(), cluster, uid=eve.new_euid)
                    self.__show_node(node)
                    if j == i: self.__highlight_node(node)
                    eve_desc = "%s<br><font color='red'><b>seteuid</b></font> to<br>%s" % \
                        (eve.pidhost(), eve.new_euid)
        
                elif eve.type == 'execve':
                    node = self.__get_node(eve.pidhost(), cluster, exe=eve.new_exe)
                    self.__show_node(node)
                    if j == i: self.__highlight_node(node)
                    eve_desc = "%s<br><font color='red'><b>execves</b></font><br>%s" % \
                        (eve.pidhost(), eve.new_exe)
        
                elif eve.type == 'exit':
                    node = self.__get_node(eve.pidhost(), cluster)
                    self.__hide_node(node)
                    eve_desc = "%s<br><font color='red'><b>exits</b></font><br>exit code: %s" % \
                        (eve.pidhost(), eve.exit_code)
        
            event_count += 1
            with open("%s/%d.e" % (dest_dir, i), "w") as ef:
                ef.write(eve_desc)
            self.__dot.write_svg("%s/%d.svg" % (dest_dir, i))
            self.__dot.write_raw("%s/%d.dot" % (dest_dir, i))

        logging.info("Total events: %d", event_count)        
        with open("%s/config.txt" % dest_dir, "w") as cf:
            cf.write("%d" % event_count)

    def __generate_legend(self, dest_dir):
        """
        Description:
        
          1. Generate exe abbreviation list $dest_dir/abbr_exe.txt
          2. Generate user color legend
          3. Generate node and edge legend
        """

        legend_dir = "%s/legend" % dest_dir
        self.__generate_abbr_exe(legend_dir)
        self.__generate_user_color(legend_dir)
        self.__generate_node_edge_legend(legend_dir)


    #
    # The following are some tiny utilities
    #
    
    def __get_node(self, pidhost, cluster, exe = None, uid = None):
        """
        Use __node_map as cache
        """
        ret_node = None
        (pid, seq, host) = pidhost.partition('@')
        trace = self.trace_map[host][pid]

        if pidhost in self.__node_map: 
            ret_node = self.__node_map[pidhost]
        else:
            initial = self.__get_exe_initial(trace.init_exe)
            new_color = self.__get_color(trace.init_euid)
            ret_node = pydot.Node(pidhost, \
                                      label = initial, \
                                      fillcolor = new_color, \
                                      style = "filled", \
                                      fontcolor = 'white')

            cluster.add_node(ret_node)
            self.__node_map[pidhost] = ret_node


        # Set node label
        if exe != None:
            trace.cur_exe = exe
            initial = self.__get_exe_initial(exe)
            ret_node.set_label(initial)
            
        # Set node color
        if uid != None:
            trace.cur_euid = uid
            new_color = self.__get_color(uid)
            ret_node.set_fillcolor(new_color)


        # Resume from highlight
        ret_node.set_penwidth(1.0)

        # Handle dedup
        tip = "pid=%s, exe=%s, user=%s, ppid=%s, parent_exe=%s" % \
            (pid, trace.cur_exe, trace.cur_euid, trace.ppid, trace.parent_exe)
        if trace.ppid in self.__dedup_dict and \
                trace.init_exe in self.__dedup_dict[trace.ppid] and \
                trace.init_euid in self.__dedup_dict[trace.ppid][trace.init_exe] and \
                trace.pid in self.__dedup_dict[trace.ppid][trace.init_exe][trace.init_euid] and\
                len(self.__dedup_dict[trace.ppid][trace.init_exe][trace.init_euid]) > 1:
            ret_node.set_shape("doublecircle")
            ret_node.set_color('black')
            tip = "pid={ "
            for pid in self.__dedup_dict[trace.ppid][trace.init_exe][trace.init_euid]:
                tip = "%s%s " % (tip, pid)
            tip = "%s}, exe=%s, user=%s, ppid=%s, parent_exe=%s" %  \
                (tip, trace.cur_exe, trace.cur_euid, trace.ppid, trace.parent_exe)
        else:    
            ret_node.set_shape("circle")
            ret_node.set_color('white')

        # Set tooltip
        ret_node.set_tooltip(tip)

        return ret_node

    def __get_edge(self, pidhost_from, pidhost_to, event_type = None):
        """
        Use self.edge_map as cache
        """
        ret_edge = None
        if pidhost_from in self.__edge_map and \
                pidhost_to in self.__edge_map[pidhost_from] and \
                event_type in self.__edge_map[pidhost_from][pidhost_to]:
            ret_edge = self.__edge_map[pidhost_from][pidhost_to][event_type]
        else:
            ret_edge = pydot.Edge(pidhost_from, pidhost_to)
            # ret_edge.set_key('%s_%s_%s', (pidhost_from, pidhost_to, event_type)) 
            self.__dot.add_edge(ret_edge)
            if pidhost_from not in self.__edge_map:
                self.__edge_map[pidhost_from] = dict()
            if pidhost_to not in self.__edge_map[pidhost_from]:    
                self.__edge_map[pidhost_from][pidhost_to] = dict()
            self.__edge_map[pidhost_from][pidhost_to][event_type] = ret_edge

        self.__show_edge(ret_edge, event_type)
        
        # Resume from highlight
        ret_edge.set_penwidth(1.0)
        ret_edge.set_color('black')

        return ret_edge

    def __get_cluster(self, host):
        """
        Use self.cluster_map as cache
        """
        if host in self.__cluster_map: return self.__cluster_map[host]
        cluster = pydot.Cluster("%d" % hash(host),label=host)
        self.__dot.add_subgraph(cluster)
        self.__cluster_map[host] = cluster
        return cluster
        
    
    def __get_exe_initial(self, exe):
        """
        Use exe_initial_map as cache
        1. Look up __exe_initial_map; if found, return, otherwise goto 3
        2. Get rid of prefix, e.g., "condor_"
        3. Use the first non-used letter in exe name as initial

        TODO(wenbin): if all letters are used, then we need other naming
        schemes, e.g., letter+number
        """
        if exe in self.__exe_initial_map: return self.__exe_initial_map[exe]

        underline_index = exe.rfind('_')
        if underline_index == -1:
            underline_index = 0
        else:
            underline_index += 1
        stripped_exe = exe[underline_index:]

        initial = stripped_exe[0].lower()
        index = 1
        while initial in self.__initial_exe_map and \
                    self.__initial_exe_map[initial] != exe:
            initial = "%s" % stripped_exe[index].lower()
            if index == len(stripped_exe)-1:
                break
            index += 1
        self.__initial_exe_map[initial] = exe
        self.__exe_initial_map[exe] = initial
        return initial


    def __get_color(self, uid):
        """
        Use uid_color_map as cache
        """
        if uid in self.__uid_color_map: return self.__uid_color_map[uid]
        self.__uid_color_map[uid] = self.__color_list[self.__color_cur]
        self.__color_cur += 1
        if self.__color_cur == len(self.__color_list): self.__color_cur = 0
        return self.__uid_color_map[uid]

    def __make_all_invisible(self):
        """
        Make all nodes and edges invisible
        """
        for nid, node in self.__node_map.items():
            node.set_style('invisible')
        for from_id in self.__edge_map.keys():
            for to_id in self.__edge_map[from_id].keys():
                for eve_type, edge in self.__edge_map[from_id][to_id].items():
                    edge.set_style('invis')

    def __show_node(self, node):
        node.set_style('filled')

    def __hide_node(self, node):
        """
        Hide node and edges that are incident on it
        """
        node.set_style('invisible')
        edge_set = self.__node_edges_map[node]
        for edge in edge_set:
            self.__hide_edge(edge)

    def __highlight_node(self, node):
        node.set_penwidth(3.0)
        node.set_color('blue')

    def __show_edge(self, edge, event_type):
        if event_type == 'fork':
            edge.set_style('dotted')
        elif event_type == 'clone':
            edge.set_style('dotted')
            edge.set_arrowhead('empty')
        elif event_type == 'connect':
            edge.set_style('solid')

    def __hide_edge(self, edge):
        edge.set_style('invis')

    def __highlight_edge(self, edge):
        edge.set_color('blue')
        edge.set_penwidth(3.0)

    def __build_node_edges_map(self):
        """
        Build __node_edges_map to assist animate exit event
        - Each node indexes a set of edges incident on it
        """
        for from_id in self.__edge_map:
            src_node = self.__node_map[from_id]
            if src_node not in self.__node_edges_map:
                self.__node_edges_map[src_node] = set([])
            for to_id in self.__edge_map[from_id].keys():
                for eve_type, edge in self.__edge_map[from_id][to_id].items():
                    dest_node = self.__node_map[to_id]
                    if dest_node not in self.__node_edges_map:
                        self.__node_edges_map[dest_node] = set([])
                    self.__node_edges_map[src_node].add(edge)
                    self.__node_edges_map[dest_node].add(edge)

    def __is_not_dup_event(self, eve):
        """
        Is this event not a duplicate event?
        """
        # We only concern fork/clone event
        if eve.type != 'fork' and eve.type != 'clone': return True

        # We don't consider fork-then-exe, connect, and seteuid event
        trace = self.trace_map[eve.host][eve.child]
        for e in trace.event_list:
            if e.type == 'execve' or \
                    e.type == 'seteuid' or \
                    e.type == 'fork' or \
                    e.type == 'clone' or \
                    e.type == 'connect':
                return True

        # Check dedup_dict
        if eve.pid not in self.__dedup_dict:
            self.__dedup_dict[eve.pid] = dict()
        if trace.init_exe not in self.__dedup_dict[eve.pid]:
            self.__dedup_dict[eve.pid][trace.init_exe] = dict()
        if trace.init_euid not in self.__dedup_dict[eve.pid][trace.init_exe]:
            self.__dedup_dict[eve.pid][trace.init_exe][trace.init_euid] = [trace.pid]
            return True

        # At this point, we skip this event
        self.__dedup_dict[eve.pid][trace.init_exe][trace.init_euid].append(trace.pid)
        return False
                    
    def __generate_abbr_exe(self, legend_dir):
        abbr_path = "%s/abbr_exe.txt" % legend_dir
        af = open(abbr_path, "w")
        af_str = ""
        for abbr, exe_name in self.__initial_exe_map.items():
            if af_str != "":
                af_str = "%s<br>%s: %s" % (af_str, abbr, exe_name)
            else:
                af_str = "%s: %s" % (abbr, exe_name)
        af.write(af_str)

    def __generate_user_color(self, legend_dir):
        color_path = "%s/color_user.svg" % legend_dir
        color_graph = pydot.Dot(graph_type='digraph', mindist=0, ranksep=0, nodesep=0)
        nodes = []
        for user, color in self.__uid_color_map.items():
            node = pydot.Node(user, \
                              style = "filled", \
                              fillcolor = color, \
                              label = user, \
                              fontcolor = "white", \
                              fontsize = 9, \
                              fixedsize='true', \
                              shape = "circle")
            nodes.append(node)
            color_graph.add_node(node)

        # XXX: assuming we only have 3 colors for now ...    
        edge = pydot.Edge(nodes[0], nodes[1], style='invis')
        color_graph.add_edge(edge)
        color_graph.write_svg(color_path)


    def __generate_node_edge_legend(self, legend_dir):
        """
        Description:

          This function will generate something like:

          multi_proc     multi_proc_legend
                               | connect
          dummy_left1    dummy_right1
              | fork           | clone
          dummy_left2    dummy_right2
        """
        shape_path = "%s/shape.svg" % legend_dir
        shape_graph = pydot.Dot(graph_type='digraph', mindist=0, ranksep=0, nodesep=0)
        single_proc = pydot.Node('single_proc', \
                              style = "filled", \
                              fillcolor = 'red', \
                              label = ' ', \
                              shape = "circle")
        single_proc_legend = pydot.Node('single_proc_legend', \
                              style = "filled", \
                              fillcolor = 'white', \
                              label = 'A process', \
                              color = 'white',\
                              fontsize = 10, \
                              shape = "box")
        shape_graph.add_node(single_proc)
        shape_graph.add_node(single_proc_legend)
        multi_proc = pydot.Node('multi_proc', \
                              style = "filled", \
                              fillcolor = 'red', \
                              label = ' ', \
                              shape = "doublecircle")
        multi_proc_legend = pydot.Node('multi_proc_legend', \
                              style = "filled", \
                              fillcolor = 'white', \
                              label = 'Multi-process', \
                              color = 'white',\
                              fontsize = 10, \
                              shape = "box")
        shape_graph.add_node(multi_proc)
        shape_graph.add_node(multi_proc_legend)
        edge = pydot.Edge(single_proc, multi_proc, style='invis')
        shape_graph.add_edge(edge)
        edge = pydot.Edge(single_proc_legend, multi_proc_legend, style='invis')
        shape_graph.add_edge(edge)


        dummy_left1 = pydot.Node('dummy_left1', \
                              style = "filled", \
                              fillcolor = 'white', \
                              label = ' ', \
                              color = 'white',\
                              shape = "none")
        shape_graph.add_node(dummy_left1)
        dummy_left2 = pydot.Node('dummy_left2', \
                              style = "filled", \
                              fillcolor = 'white', \
                              label = ' ', \
                              color = 'white',\
                              shape = "none")
        shape_graph.add_node(dummy_left2)
        dummy_right1 = pydot.Node('dummy_right1', \
                              style = "filled", \
                              fillcolor = 'white', \
                              label = ' ', \
                              color = 'white',\
                              shape = "none")
        shape_graph.add_node(dummy_right1)
        dummy_right2 = pydot.Node('dummy_right2', \
                              style = "filled", \
                              fillcolor = 'white', \
                              label = ' ', \
                              color = 'white',\
                              shape = "none")
        shape_graph.add_node(dummy_right2)
        edge = pydot.Edge(multi_proc, dummy_left1, style='invis')
        shape_graph.add_edge(edge)
        edge = pydot.Edge(dummy_left1, dummy_left2, label='fork', style='dotted', fontsize=10)
        shape_graph.add_edge(edge)
        edge = pydot.Edge(multi_proc_legend, dummy_right1, style='solid', label='connect', fontsize=10)
        shape_graph.add_edge(edge)
        edge = pydot.Edge(dummy_right1, dummy_right2, style='dotted', label='clone', arrowhead='empty', fontsize=10)
        shape_graph.add_edge(edge)
        shape_graph.write_svg(shape_path)


