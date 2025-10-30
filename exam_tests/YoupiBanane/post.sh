#!/bin/bash
# --- valid CGI headers (CRLF required) ---
echo -en "Content-Type: text/plain\r\n\r\n"

# --- echo the POST body directly ---
# 'cat' reads stdin (the POST data) and outputs it verbatim
cat
