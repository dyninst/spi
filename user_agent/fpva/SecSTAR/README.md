SecSTAR
=======

Visualize system structure and security property. There are two components:

- SecSTAR/parser: Python scripts for parsing xml trace files to svg images
```
usage: SecSTAR.py [-h] --input_dir INPUT_DIR --output_dir OUTPUT_DIR

optional arguments:
  -h, --help            show this help message and exit
  --input_dir INPUT_DIR
                        DIR contains trace xml files
  --output_dir OUTPUT_DIR
                        DIR for output svg images
```

- SecSTAR/html: Javascript to animate svg files
  - The tricky part here is how to load all svg files efficiently. The solution is to load a zip file, then unzip it in browser and split all svgs.
