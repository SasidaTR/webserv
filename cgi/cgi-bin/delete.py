#!/usr/bin/env python3
import os
import sys

method = os.environ.get('REQUEST_METHOD', 'GET')

filepath = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'html', 'files', 'test.txt')

if method == 'DELETE':
    if os.path.exists(filepath):
        try:
            os.remove(filepath)
        except:
            pass
