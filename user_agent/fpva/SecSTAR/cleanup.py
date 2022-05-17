import os
import xml.etree.ElementTree as ET
import xml.dom.minidom

directory = os.fsencode("./tmp/spi")
file_pids = {}

for file in os.listdir(directory):
	filename = os.fsdecode(file)
	if filename.endswith(".xml"):
		pid = filename.split('-')[0]
		if pid not in file_pids.keys():
			file_pids[pid] = []
		file_pids[pid].append(filename)

for pid in file_pids:
	try:
		primary = ''
		for filename in file_pids[pid]:
			file = ET.parse("./tmp/spi/" + filename)
			exe = file.findall(".//exe_name")
			if len(exe):
				primary = filename
				file_pids[pid].remove(filename)
				break
	
		primary_file = ET.parse("./tmp/spi/" + primary)
		root = primary_file.getroot()	
		primary_traces = root.find('traces')
		for filename in file_pids[pid]:
			file = ET.parse("./tmp/spi/" + filename)
			traces = file.findall(".//trace")
			print(len(traces))
			for trace in traces:
				primary_traces.append(trace)
	
		primary_file.write(pid + ".xml")	
		#pretty = xml.dom.minidom.parseString(ET.tostring(root)).toprettyxml(indent=" ", newl='\n')
		#f = open(pid + ".xml", "w")
		#res = f.write(pretty)
		#f.close()
	except:
		print("Exception occured (is there an exe_name for every pid?)")
