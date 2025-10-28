#!/usr/bin/env python3
import os, sys

# --- CGI headers ---
print("Status: 200 OK")
print("Content-Type: text/plain")
print()

print("=== Python CGI POST Test ===")
print(f"REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', '')}")
print(f"CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', '')}")
print(f"CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', '')}")
print(f"QUERY_STRING: {os.environ.get('QUERY_STRING', '')}")
print(f"SCRIPT_NAME: {os.environ.get('SCRIPT_NAME', '')}")
print()

# --- Read body from stdin ---
try:
    length = int(os.environ.get("CONTENT_LENGTH", 0) or 0)
except ValueError:
    length = 0

if length > 0:
    body = sys.stdin.read(length)
    print("--- BODY START ---")
    print(body)
    print("--- BODY END ---")
else:
    print("(No body received)")

print("\n=== END ===")
