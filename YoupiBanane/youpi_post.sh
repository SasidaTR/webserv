#!/bin/bash
# Simple CGI script that echoes back POST data

# --- CGI headers ---
echo "Content-Type: text/plain"
echo

# --- Read POST body from stdin ---
if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
    read -n "$CONTENT_LENGTH" POST_DATA
else
    POST_DATA=""
fi

# --- Print a response ---
echo "Hello from CGI!"
echo "You sent: $POST_DATA"
