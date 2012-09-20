"""
Combine XML files of the same process into a single XML file 
"""

import logging
import os

from structures import trace

def run(input_dir):
    """
    Description:

      1. If $pid.xml not exist, move the first $pid-$seq.xml to $pid.xml.
      2. If $pid.xml exists, insert things in between <traces></traces> of other
      $pid-$seq.xmls into $pid.xml.
      3. If any trace file doesn't contain '-', then we assume all trace files
      are combined, we return immediately.

    Args:

      input_dir: the root dir path of trace xml file. Assuming that the dir
      structure is something like:
        - $input_dir/
          - host 1/
            - xml files
          - host 2/
            - xml files
          - host n/
    """

    logging.info('[1] CombineWorker is running for %s', input_dir)
    for host_dir in os.listdir(input_dir):
        host_path = os.path.join(input_dir, host_dir)
        for xml_file in sorted(os.listdir(host_path)):
            (pid, sep, tmp_seq) = xml_file.partition("-")
            if sep == "": 
                logging.info("[1] Already combined, skip this worker")
                return
            (seq, sep, subfix) = tmp_seq.partition(".")
            xml_path = os.path.join(host_path, xml_file)
            new_xml_path = os.path.join(host_path, "%s.xml" % pid)

            if os.path.exists(new_xml_path):
                # Extract <trace>'s in between <traces></traces> into $pid.xml
                logging.info("[1] Appending %s to %s", xml_path, new_xml_path)
                to_trace = trace.Trace(new_xml_path)
                to_trace.insert_traces_from(xml_path)
                logging.info("[1] Removing %s", xml_path)
                os.remove(xml_path)
            else:
                logging.info("[1] Rename %s to %s", xml_path, new_xml_path)
                os.rename(xml_path, new_xml_path)
