How do the SPI data collector and Python script visualizer cooperate?
------------------------------------------------------------------------------
SPI Data Collector
- For each process, produce two types of files:
  1. /tmp/$pid-seq-num, recording only a integer $seq. The $seq is incremented by 1 in these cases:
    1.1. fork
    1.2. clone
    1.3. exec
  2. /tmp/$pid-$seq.xml, where $seq is read from /tmp/$pid-seq-num

Python script visualizer
- Combine all $pid-$seq.xmls that have the same $pid into one single xml file
  1.1. Copy $pid-1.xml to be $pid.xml
  1.2. Append things in between <traces></traces> of other $pid-$seq.xmls into $pid.xml's <traces></traces>
  1.3. This can be done using the python helper script SecSTAR/cleanup.py, which merges .xml traces found locally in a tmp/spi/ directory. Place cleanup.py in your SPI_DIR or else edit this path to traces/host/.

- Place .xml traces inside the /SecSTAR/traces/ directory. Separate traces into directories by host, i.e. /traces/host/12345.xml for data from a single host.  
