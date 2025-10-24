#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print()

filepath = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(__file__))),
    'html', 'files', 'index2.html'
)

try:
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    print(content)
except FileNotFoundError:
    print("<h1>Fichier introuvable</h1>")
except Exception as e:
    print("<h1>Erreur lors de la lecture du fichier</h1>")
    print("<p>{}</p>".format(e))
