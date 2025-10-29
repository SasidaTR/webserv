#!/usr/bin/env python3
import os
import glob

print("Content-Type: text/plain\r\n\r\n")

try:
	files_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../html/files"))

	if not os.path.isdir(files_dir):
		print(f"Erreur : dossier {files_dir} inexistant")
	else:
		txt_files = sorted(glob.glob(os.path.join(files_dir, "*.txt")))

		if not txt_files:
			print("Aucun fichier .txt trouvé")
		else:
			for fpath in txt_files:
				try:
					with open(fpath, "r") as f:
						print(f.read())
						print()
				except Exception as e:
					print(f"Erreur lecture fichier {fpath}: {e}")
except Exception as e:
	print(f"Erreur générale du script: {e}")
