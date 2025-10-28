#!/bin/sh

# Read all stdin and count bytes
count=$(cat | wc -c)

# Minimal valid CGI response:
echo "Content-Type: text/plain"
echo
echo "CGI received $count bytes"
