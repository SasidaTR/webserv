#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\r")
print("\r")
print("<html><head><title>POST CGI Test</title></head><body>")
print("<h1>POST Data Received</h1>")

method = os.environ.get('REQUEST_METHOD', 'GET')
print(f"<p><strong>Method:</strong> {method}</p>")

if method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<p><strong>POST Data:</strong> {post_data}</p>")
    else:
        print("<p><strong>POST Data:</strong> No data received</p>")

query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print(f"<p><strong>Query String:</strong> {query_string}</p>")

print("</body></html>")