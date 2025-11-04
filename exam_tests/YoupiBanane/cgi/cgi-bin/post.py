#!/usr/bin/python3
import sys, pathlib, os, time

length = int(os.environ.get("CONTENT_LENGTH") or 0)
body = sys.stdin.buffer.read(length) if length > 0 else b""

save_dir = pathlib.Path(__file__).parent.parent.parent / "html" / "files"
timestamp = time.strftime("%Y%m%d_%H%M%S")
filename = save_dir / f"post_{timestamp}.txt"

with open(filename, "wb") as f:
	f.write(body)

print("Content-Type: text/plain")
print()
print("OK")
