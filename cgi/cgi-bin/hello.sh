#!/bin/bash

echo "Content-Type: text/html"
echo ""
echo "<html>"
echo "<head><title>Bash CGI Test</title></head>"
echo "<body>"
echo "<h1>Hello from Bash CGI!</h1>"
echo "<p><strong>Request Method:</strong> $REQUEST_METHOD</p>"
echo "<p><strong>Query String:</strong> $QUERY_STRING</p>"
echo "<p><strong>Date:</strong> $(date)</p>"
echo "<p><strong>Server Info:</strong> $SERVER_NAME:$SERVER_PORT</p>"
echo "</body>"
echo "</html>"