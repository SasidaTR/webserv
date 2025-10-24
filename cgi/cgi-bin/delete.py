#!/usr/bin/env python3
import os
import sys

method = os.environ.get('REQUEST_METHOD', 'GET')

filepath = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'html', 'files', 'test.txt')

print("Content-Type: text/plain\n")

if method == 'DELETE':
    if os.path.exists(filepath):
        try:
            os.remove(filepath)
            print("File deleted successfully")
        except Exception as e:
            print("Error deleting file:", e)
    else:
        print("File does not exist")
else:
    print("Unsupported method:", method)
