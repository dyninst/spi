"""
Lookup table for raw traces and events
"""

class LookupTable:
    """
    Args:
    
      trace_map: an associative array of traces, each of which contains
      information of a process. The look up key is (host, pid)

      event_list: a list of events sorted by time
    """
    def __init__(self):
        self.trace_map = dict()
        self.event_list = list()

    def insert_trace(self, trace):
        # Lazy parsing
        trace.parse()
        host = "%s" % trace.hostname
        pid = "%s" % trace.pid

        self.__add_to_2d_dict(self.trace_map, host, pid, trace)

    def build_event_list(self):
        """
        Build event list by the following steps:
        1. Add every event in every trace into event_list
        2. For connect event, get target pid, and update this event
        3. Get rid of these events that involve a process not in trace_map's
           keys or not have any relationship with other process
        4. Sort event list
        """

        relation_map = dict()
        relation_types = ['connect', 'fork', 'clone', 'accept']
        for host in self.trace_map.keys():
            for pid, trace in self.trace_map[host].items():
                for eve in trace.event_list:
                    if eve == None: continue
                    if eve.type == 'connect':
                        trg_pid = self.__get_pid_by_port(eve.trg_host, eve.trg_port)
                        if trg_pid == -1: continue
                        eve.set_trg_pid(trg_pid)

                    # Add every events into event_list first
                    if eve.type != 'accept':
                        self.event_list.append(eve)

                    # Build relation map, which records the process involved
                    # in relationships
                    if eve.type in relation_types:
                        self.__add_to_2d_dict(relation_map, host, pid, 1)
                        if eve.type == 'connect' and \
                                eve.trg_host in self.trace_map and \
                                eve.trg_pid in self.trace_map[eve.trg_host]:
                            self.__add_to_2d_dict(relation_map, eve.trg_host, eve.trg_pid, 1)
                        elif (eve.type == 'fork' or eve.type == 'clone') and \
                                eve.host in self.trace_map and \
                                eve.child in self.trace_map[eve.host]:
                            self.__add_to_2d_dict(relation_map, eve.host, eve.child, 1)

        # We only keep those processes that have relationships with othersx
        self.event_list = [eve for eve in self.event_list \
                                   if eve.host in relation_map and \
                                   eve.pid in relation_map[eve.host]]

        # Sorting!
        self.event_list.sort()
        # for e in self.event_list: print e

    def __get_pid_by_port(self, host, port):
        if host not in self.trace_map: return -1
        for pid, trace in self.trace_map[host].items():
            if trace.has_port(port) == True:
                return pid
        return -1


    def __is_legal_event(self, eve):
        pass


    def __add_to_2d_dict(self, the_dict, key1, key2, val):
        """
          The right way to add value to a 2d dictionay
        """

        try:
            the_dict[key1]
        except KeyError, e:
            the_dict[key1] = {}

        try:
            the_dict[key1][key2]
        except KeyError, e:
            the_dict[key1][key2] = {}

        the_dict[key1][key2] = val
