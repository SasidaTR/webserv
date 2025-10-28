#!/usr/bin/python3
import sys, time, pathlib, os

length = int(os.environ.get("CONTENT_LENGTH") or 0)
body = sys.stdin.buffer.read(length) if length > 0 else b""

save_dir = pathlib.Path(__file__).parent.parent.parent / "html" / "files"

timestamp = time.strftime("%Y%m%d_%H%M%S")
filename = save_dir / f"post_{timestamp}.txt"

try:
	with open(filename, "wb") as f:
		f.write(body)
	msg = f"Saved POST body to: {filename.name}"
except Exception as e:
	msg = f"Error saving file: {str(e)}"

html = f"""<html><body>
<h1>CGI OK</h1>
<p>{msg}</p>
<pre>{body.decode('utf-8', 'replace')}</pre>
</body></html>"""

out = html.encode("utf-8")

print("Status: 200 OK")
print("Content-Type: text/html; charset=utf-8")
print(f"Content-Length: {len(out)}")
print()
sys.stdout.buffer.write(out)
