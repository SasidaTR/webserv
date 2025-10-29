#!/bin/bash
echo "Content-Type: text/plain"
echo

# Read the request body from stdin
if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
    read -n "$CONTENT_LENGTH" POST_DATA
else
    POST_DATA=""
fi

echo "Hello from CGI!"
echo "You sent: $POST_DATA"
