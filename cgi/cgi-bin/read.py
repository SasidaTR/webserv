#!/usr/bin/env python3
import os

filepath = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'html', 'files', 'test.txt')

print("Content-Type: text/plain")
print()

if os.path.exists(filepath):
    with open(filepath, "r") as f:
        print(f.read())
else:
    print("Aucun fichier trouvé")
