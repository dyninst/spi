"""
Build lookup tables to hold traces and event list
"""

import logging
import os

from structures import trace
from structures import lookup_table

def run(input_dir):
    """
    Description:
      1. For each xml trace file $pid.xml, build a Trace object
      2. Feed all Trace objects into LookupTable object
         - Inside LookupTable, it automatically build up event list

    Args:

      input_dir: the root dir path of trace xml file. Assuming that the dir
      structure is something like:
        - $input_dir/
          - host 1/
            - xml files
          - host 2/
            - xml files
          - host n/

    Return:
      
      A lookup table holding everything about traces
    """

    logging.info('[2] LookupTableWorker is running for %s', input_dir)

    table = lookup_table.LookupTable()
    for host_dir in os.listdir(input_dir):
        host_path = os.path.join(input_dir, host_dir)
        for xml_file in sorted(os.listdir(host_path)):
            (pid, sep, suffix) = xml_file.partition(".")
            xml_path = os.path.join(host_path, xml_file)
            the_trace = trace.Trace(xml_path)
            table.insert_trace(the_trace)
#            print the_trace

    table.build_event_list()
    return table
