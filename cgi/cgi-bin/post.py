#!/usr/bin/python3
import os, sys, time, pathlib

def read_body():
    try:
        l = int(os.environ.get("CONTENT_LENGTH") or 0)
    except ValueError:
        l = 0
    return sys.stdin.buffer.read(l) if l > 0 else b""

method = os.environ.get("REQUEST_METHOD", "GET")
ctype  = os.environ.get("CONTENT_TYPE", "")
qs     = os.environ.get("QUERY_STRING", "")
body   = read_body()

save_dir = pathlib.Path(__file__).parent / "files"
save_dir.mkdir(parents=True, exist_ok=True)

timestamp = time.strftime("%Y%m%d_%H%M%S")
filename = save_dir / f"post_{timestamp}.txt"

try:
    with open(filename, "wb") as f:
        f.write(body)
    saved_ok = True
except Exception as e:
    saved_ok = False
    error_msg = str(e)

msg = f"Saved POST body to: {filename}" if saved_ok else f"Error saving file: {error_msg}"

html = f"""<html><body>
<h1>CGI OK</h1>
<p>Method: {method}</p>
<p>Content-Type: {ctype}</p>
<p>{msg}</p>
<p>Body bytes: {len(body)}</p>
<pre>{body.decode('utf-8', 'replace')}</pre>
</body></html>"""

out = html.encode("utf-8")

print("Status: 200 OK")
print("Content-Type: text/html; charset=utf-8")
print(f"Content-Length: {len(out)}")
print()
sys.stdout.flush()
sys.stdout.buffer.write(out)
sys.stdout.flush()
