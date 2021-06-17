SecSTAR
=======
Visualize system structure and security property. There are two components:
- SecSTAR/SecSTAR: Python scripts (require: pydot module)
  - input: xml trace data in SecSTAR/traces
  - output: svg files
- SecSTAR/html: Javascript to animate svg files
  - The tricky part here is how to load all svg files efficiently. The solution is to load a zip file, then unzip it in browser and split all svgs.
