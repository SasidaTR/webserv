#!/usr/bin/env python3
import os, sys, glob

method = os.environ.get('REQUEST_METHOD', 'GET')
files_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../html/files"))

print("Content-Type: text/plain\n")

if method == 'DELETE':
	txt_files = glob.glob(os.path.join(files_dir, "*.txt"))
	if not txt_files:
		print("No .txt files to delete")
	else:
		latest_file = max(txt_files, key=os.path.getmtime)
		try:
			os.remove(latest_file)
			print(f"Deleted file: {os.path.basename(latest_file)}")
		except Exception as e:
			print("Error deleting file:", e)
else:
	print("Unsupported method:", method)
