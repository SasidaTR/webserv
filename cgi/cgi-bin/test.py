#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\r")
print("\r")
print("<html>")
print("<head><title>CGI Test Script</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")

env_vars = ['REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH', 
           'SERVER_NAME', 'SERVER_PORT', 'SCRIPT_NAME', 'PATH_INFO']

for var in env_vars:
    value = os.environ.get(var, 'Not set')
    print(f"<li><strong>{var}:</strong> {value}</li>")

print("</ul>")
print("<h2>All Environment Variables:</h2>")
print("<pre>")
for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")
print("</pre>")
print("</body>")
print("</html>")