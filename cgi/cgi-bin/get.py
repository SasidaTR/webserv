#!/usr/bin/env python3
import os

print("Content-Type: text/html")
print()

filepath = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'html', 'files', 'index2.html')

if os.path.exists(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    print(content)
else:
    print("<h1>Fichier introuvable</h1>")
