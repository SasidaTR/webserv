#!/usr/bin/env python3
import os
import sys

method = os.environ.get('REQUEST_METHOD', 'GET')
files_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'html', 'files')
files_dir = os.path.abspath(files_dir)

print("Content-Type: text/plain\n")

if method == 'DELETE':
    txt_files = [f for f in os.listdir(files_dir) if f.endswith('.txt')]
    if not txt_files:
        print("No .txt files to delete")
    else:
        txt_files_paths = [os.path.join(files_dir, f) for f in txt_files]
        latest_file = max(txt_files_paths, key=os.path.getmtime)
        try:
            os.remove(latest_file)
            print(f"Deleted file: {os.path.basename(latest_file)}")
        except Exception as e:
            print("Error deleting file:", e)
else:
    print("Unsupported method:", method)
