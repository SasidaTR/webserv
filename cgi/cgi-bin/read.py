#!/usr/bin/env python3
import os

filepath = os.path.join(os.path.dirname(__file__), "../../html/files/test.txt")
filepath = os.path.abspath(filepath)

print("Content-Type: text/plain")
print()

if os.path.exists(filepath):
    try:
        with open(filepath, "r") as f:
            print(f.read())
    except Exception as e:
        print(f"Erreur lecture fichier: {e}")
else:
    print("Aucun fichier trouvé")
