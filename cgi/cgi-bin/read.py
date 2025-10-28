#!/usr/bin/env python3
import os
import glob

files_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../html/files"))

print("Content-Type: text/plain")
print()

txt_files = sorted(glob.glob(os.path.join(files_dir, "*.txt")))

if txt_files:
    for fpath in txt_files:
        try:
            with open(fpath, "r") as f:
                print(f"--- {os.path.basename(fpath)} ---")
                print(f.read())
                print()
        except Exception as e:
            print(f"Erreur lecture fichier {fpath}: {e}")
else:
    print("Aucun fichier .txt trouvé")
