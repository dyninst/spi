"""
Trace: Parse and represents an XML trace file
"""

import logging
import os
import re
import sys

from xml.dom.minidom import parse

from structures import event

class Trace:
    """
    Summary:
      
      Parse an XML trace file and provide getters for information

    Attributes:

      file_path: xml trace file's path

      hostname
      pid
      ppid: parent process's pid
      parent_exe: parent process's exe
      init_exe
      init_euid: effective user name (not the numerical one!)
      init_working_dir
      cur_exe: current exe
      cur_euid: current user name
      cur_working_dir
      event_list: A list of Event objects. These events are supported:
            - fork
            - clone
            - connect
            - accept
            - seteuid
            - open
            - chown
            - chdir, chroot
            - exit
      port_list: a list of ports that are open to accect connections
    """

    def __init__(self, file_path):
        """
        Summary:
        
          Open trace file and parse it immediately

        Args:
         
          file_path: the full or relative path for the XML trace file
        """
        self.file_path = file_path
        self.hostname = ''
        self.pid = ''
        self.ppid = ''
        self.parent_exe = ''
        self.init_exe = ''
        self.init_euid = ''
        self.init_working_dir = ''
        self.cur_exe = ''
        self.cur_euid = ''
        self. cur_working_dir = ''
        self.event_list = []
        self.port_list = set([])

    def insert_traces_from(self, from_path):
        """
        Summary:
          
          Get text of <traces>...</traces> from `from_trace`, then insert it
          right before </traces> in `self`.

          This is the step for combining multiple trace files into a single one.

        Args:
         
          from_path: the xml trace file path to be combined into `self`
        """
        to_file = open(self.file_path, 'r+')
        from_file = open(from_path, 'r')
        from_text = from_file.read()
        m = re.search('.+<traces>(.*)</traces></process>', from_text)
        if m == None: return
        traces_text = m.group(1)
        traces_text += '</traces></process>'
        to_file.seek(-19, 2)
        to_file.write(traces_text)

    def parse(self):
        """
        Summary:

          Parse current trace xml file, the following information will be
          obtained:
          
          - Hostname
          - PID
          - init_exe
          - init_euid
          - init_working_dir
          - cur_exe
          - cur_euid
          - cur_working_dir
          - Event List, possible events include
            - fork
            - clone
            - connect
            - accept
            - seteuid
            - open
            - chown
            - chdir, chroot
            - exit
            - execs
            - send
            - recv
        """

        logging.info("- Parsing %s", self.file_path)

        dom = parse(self.file_path)
        head_nodes = dom.getElementsByTagName('head')[0].childNodes
        self.hostname = self.__get_node_value1(head_nodes, 'host')
        self.pid = self.__get_node_value1(head_nodes, 'pid')
        self.init_exe = os.path.basename(self.__get_node_value1(head_nodes,
                                                                'exe_name'))
        self.cur_exe = self.init_exe
        self.init_working_dir = self.__get_node_value1(head_nodes, 'working_dir')
        self.cur_working_dir = self.init_working_dir
        self.init_euid = self.__get_node_value2(head_nodes, 'effective_user', 'name')
        self.cur_euid = self.init_euid
        self.ppid = self.__get_node_value2(head_nodes, 'parent', 'pid')
        try:
            self.parent_exe = os.path.basename(self.__get_node_value2(head_nodes,
                                                                  'parent',
                                                                  'exe_name'))
        except:
            print("Skipping parent exe name")

        traces_nodes = dom.getElementsByTagName('traces')[0].childNodes
        for eve in traces_nodes:
            eve_type = self.__get_attr_value1(eve, 'type')
            eve_time = self.__get_attr_value1(eve, 'time')
            the_eve = None
            if eve_type == 'fork':
                # XXX: abnormal forked pid? skip it for now
                if  eve.firstChild.nodeValue == '4294967295': continue
                the_eve = event.ForkEvent(eve_type, eve_time, self.hostname,
                                          self.pid, self.__get_node_value1(eve.childNodes, 'pid'))
            elif eve_type == 'clone':
                the_eve = event.CloneEvent(eve_type, eve_time, self.hostname,
                                           self.pid, self.__get_node_value1(eve.childNodes, 'pid'))
            elif eve_type == 'connect':
                trg_host = self.__get_node_value1(eve.childNodes, 'host')
                if trg_host == '127.0.0.1':
                    trg_host = self.hostname
                trg_port = self.__get_node_value1(eve.childNodes, 'port')
                the_eve = event.ConnectEvent('connect', eve_time, self.hostname,
                                             self.pid, trg_host, trg_port)

            elif eve_type == 'send':
                trg_host = self.__get_node_value1(eve.childNodes, 'host')
                if trg_host == '127.0.0.1':
                    trg_host = self.hostname
                trg_pid = self.__get_node_value1(eve.childNodes, 'pid')
                the_eve = event.SendEvent('send', eve_time, self.hostname,
                                             self.pid, trg_host, trg_pid)

            elif eve_type == 'recv':
                trg_host = self.__get_node_value1(eve.childNodes, 'host')
                if trg_host == '127.0.0.1':
                    trg_host = self.hostname
                trg_port = self.__get_node_value1(eve.childNodes, 'port')
                the_eve = event.RecvEvent('recv', eve_time, self.hostname,
                                             self.pid, trg_host, trg_port)

            elif eve_type == 'accept':
                trg_host = self.__get_node_value1(eve.childNodes, 'host')
                if trg_host == '127.0.0.1':
                    trg_host = self.hostname
                trg_port = self.__get_node_value1(eve.childNodes, 'port')
                the_eve = event.AcceptEvent('accept', eve_time, self.hostname,
                                            self.pid, trg_host, trg_port)
                self.port_list.add(trg_port)

            elif eve_type == 'exit':
                exit_code = eve.firstChild.nodeValue
                the_eve = event.ExitEvent('exit', eve_time, self.hostname,
                                          self.pid, exit_code)
            elif eve_type == 'seteuid':
                new_euid = self.__get_node_value1(eve.childNodes, 'name')
                the_eve = event.SeteuidEvent('seteuid', eve_time, self.hostname,
                                             self.pid, new_euid)

            elif eve_type == 'execve' or eve_type == 'execv' or eve_type == 'execvpe' or eve_type == 'fexecve' or eve_type == 'execveat' or eve_type == 'execle' or eve_type == 'execl' or eve_type == 'execlp' or eve_type == 'execvp':
                new_exe = os.path.basename(self.__get_node_value1(eve.childNodes, 'path'))
                argvs = self.__get_node_value1 (eve.childNodes, 'argvs');
                envs = self.__get_node_value1 (eve.childNodes, 'envs');
                directory_fd = self.__get_node_value1 (eve.childNodes, 'directory_fd');
                flags = self.__get_node_value1 (eve.childNodes, 'flags');
                exe_type = eve_type
                the_eve = event.ExeEvent('exec', eve_time, self.hostname,
                                          self.pid, new_exe, exe_type, argvs, envs, directory_fd, flags)

            elif eve_type == 'open' or eve_type == 'fopen' or eve_type == 'fdopen' or eve_type == 'freopen' or eve_type == 'openat':
                the_file = self.__get_node_value1(eve.childNodes, 'file')
                mode = self.__get_node_value1(eve.childNodes, 'mode')
                flags = self.__get_node_value1(eve.childNodes, 'flags')
                directory_fd = self.__get_node_value1(eve.childNodes, 'directory_fd')
                open_type = eve_type
                the_eve = event.OpenEvent('open', eve_time, self.hostname,
                                         self.pid, the_file, open_type, mode, directory_fd, flags)
                print("FILE1: " + eve_type)
                if (the_file != None): print("FILE2: " + the_file)
            
            elif eve_type == 'chown' or eve_type == 'fchown' or eve_type == 'lchown' or eve_type == 'fchownat':
                the_file = self.__get_node_value1(eve.childNodes, 'file')
                directory_fd = self.__get_node_value1(eve.childNodes, 'directory_fd')
                owner = self.__get_node_value1(eve.childNodes, 'owner')
                group = self.__get_node_value1(eve.childNodes, 'group')
                flags = self.__get_node_value1(eve.childNodes, 'flags')
                chfile_type = eve_type
                the_eve = event.FileEvent('chown', eve_time, self.hostname,
                                         self.pid, the_file, chfile_type, owner, group, directory_fd, flags)

            elif eve_type == 'chroot' or eve_type == 'chdir':
                new_directory = self.__get_node_value1(eve.childNodes, 'directory')
                the_eve = event.DirectoryEvent(eve_type, eve_time, self.hostname,
                                              self.pid, new_directory)
                print("CH " + new_directory)

            self.event_list.append(the_eve)

    def __get_node_value1(self, nodes, key):
        """
        Summary:

          Get $val for such node: <nodes><key>$val</key></nodes>

        Args:

          nodes: a list of xml nodes
          key: the search key
        """
        for node in nodes:
            if (node.nodeName == key):
                trg_node = node.firstChild
                return trg_node.nodeValue
        return None

    def __get_node_value2(self, nodes, key1, key2):
        """
        Summary:

          Get $val for such node: <nodes><key1>$val<key2></key2></key1></nodes>

        Args:

          nodes: a list of xml nodes
          key1: the key1
          key2: the key2
        """
        for node1 in nodes:
            if (node1.nodeName == key1):
                trg_nodes = node1.childNodes
                for node2 in trg_nodes:
                    if node2.nodeName == key2:
                        trg_node = node2.firstChild
                        return trg_node.nodeValue
        return None


    def __get_attr_value1(self, node, attr):
        """
        Summary:

          Get $val for such node: <nodes><key attr=val></key></nodes>
        """

        if (node.nodeName == 'trace'):
            val = node.getAttribute(attr)
            return val
        return None

    def __str__(self):
        ret = ''
        ret += 'Hostname: %s\n' % self.hostname
        ret += 'PID: %s\n' % self.pid
        ret += 'Init Exe: %s\n' % self.init_exe
        ret += 'Init Euid: %s\n' % self.init_euid
        ret += 'Init Working Directory: %s\n' % self.init_working_dir
        ret += 'Events:\n'
        for eve in self.event_list:
            ret = '%s%s' % (ret, eve)
        return ret

    def has_port(self, port):
        if port in self.port_list: 
            return True
        return False

