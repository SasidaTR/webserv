# cgi/cgi-bin/showenv.sh
#!/bin/sh
echo "Content-Type: text/plain"
echo
echo "SCRIPT_FILENAME=$SCRIPT_FILENAME"
echo "PATH_INFO=$PATH_INFO"
echo "PATH_TRANSLATED=$PATH_TRANSLATED"
echo "SCRIPT_NAME=$SCRIPT_NAME"
echo "FOO=$FOO"
