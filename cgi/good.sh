#!/bin/bash
# note the explicit \r for Windows-style CRLF line endings
echo -en "Content-Type: text/plain\r\n\r\n"
echo "Hello from good CGI"
