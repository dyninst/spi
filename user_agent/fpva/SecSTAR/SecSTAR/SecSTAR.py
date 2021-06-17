"""
SecSTAR visualizer: Processing xml trace data and constructing svg images.

The class relationships:
- GraphBuilder uses
  - CombineWorker
  - LookupTableWorker
  - TemporalListWorker
  - StaticGraphWorker
  - AnimationWorker
"""

import logging
import os

from workers import combine_worker
from workers import lookup_table_worker
from workers import static_graph_worker
from workers import animation_worker

class GraphBuilder:
    """
    Summary:

      The builder class to generate graphs.

    Description:

      The go() method is the only public method to parse XML trace data and
      generate static graph and animation graphs. The detailed steps involve:
      1. Combine multiple xml files of the same process into one single file;
      2. Parse those single XML files and build a lookup table to store
      information in memory;
      3. Build a list of temporal events from the lookup table;
      4. Generate static graph;
      5. Generate animation graphs.

    Attributes:
    """

    def __init__(self):
        pass

    def package(self, dir_path, dedup = False):
        """
        Summary:

           Pack svgs / events into a file and zip them
           1. Zip static.svg
              - output: static.zip
              - delimiter: "<?xml ...>" (default)
           2. Cancatenate all svgs, zip them
              - output: svgs.zip
              - delimiter: "<?xml ...>" (default)
           3. Cancatenate all events, zip them
              - output: events.zip
              - delimiter: "===event===\n" (additional)
        """
        path = "%s/raw" % dir_path
        if dedup:
            path = "%s/dedup" % dir_path

        logging.info(path)
        logging.info("Remove all .dot files")
        os.system("rm -f %s/*.dot" % path)
        os.system("rm -f %s/*.zip" % path)

        if not os.path.exists("%s/static.zip" % path):
            logging.info("Package static.svg")
            os.system("cd %s;zip -9 static.zip static.svg" % path)
            os.system("rm -f %s/static.svg" % path)

        if not os.path.exists("%s/svgs.zip" % path):
            logging.info("Package animation svg files")
            os.system("cd %s;ls *.svg | sort -n | xargs cat >> svgs.svg" % path)
            os.system("cd %s; zip -9 svgs.zip svgs.svg" % path)
            os.system("rm -f %s/*.svg" % path)

        if not os.path.exists("%s/events.zip" % path):
            logging.info("Package event files")
            os.system("cd %s;ls *.e | sort -n >> event_tmp" % path)
            fp = open("%s/event_tmp" % path, "r")
            for file_name in fp:
                file_name = file_name.rstrip()
                os.system("echo \"\n===event===\n\" >> %s/events.e" % path)
                os.system("cat %s/%s >> %s/events.e" % (path, file_name, path))
            fp.close()
            os.system("cd %s;zip -9 events.zip events.e" % path)
            os.system("rm -f %s/event_tmp" % path)
            os.system("rm -f %s/*.e" % path)

    def go(self, input_dir, output_dir, dedup = False):
        """
        Summary:

          The only public method in this class. It deploys a list of workers to
          build graphs. Each invocation is idempotent.

          If any worker fails, the program stops immediately. FAIL-STOP policy.

        Args:

          input_dir: The root dir path for xml trace data. It can be either
          relative path or absolute path.

          output_dir: The root dir path for generated graphs. It can be either
          relative path or absolute path.

          groupping: Whether or not to group those leaf nodes that have the same
          parent, the same privilege level, and the same executable. Default
          value is False.
        """
        logging.info('Start working ...')

        # Step 1
        combine_worker.run(input_dir)
        
        # Step 2
        table = lookup_table_worker.run(input_dir)

        # Step 3
        static_graph_worker.run(table, output_dir, dedup)

        # Step 4
        animation_worker.run(table, output_dir, dedup)
        
def main():
    logging.basicConfig(level=logging.INFO)

    input_dir = '../traces'
    output_dir = '../html/images'
  
    builder = GraphBuilder()
    builder.go(input_dir, output_dir, dedup=False)
#    builder.go(input_dir, output_dir, dedup=True)
    builder.package(output_dir, dedup=False)
#    builder.package(output_dir, dedup=True)

if __name__ == "__main__":
    main()
