#!/usr/bin/env python3
import os
import sys

method = os.environ.get('REQUEST_METHOD', 'GET')

if method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length) if content_length > 0 else ''

    filepath = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'html', 'files', 'test.txt')
    with open(filepath, "w") as f:
        f.write(post_data)
