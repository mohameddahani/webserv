#!/usr/bin/env python3
import os
import sys

# Read environment variables
method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
content_type = os.environ.get('CONTENT_TYPE', 'UNKNOWN')
content_length = os.environ.get('CONTENT_LENGTH', '0')
query_string = os.environ.get('QUERY_STRING', '')

# Read POST data from stdin
post_data = ''
if method == 'POST':
    try:
        length = int(content_length)
        post_data = sys.stdin.read(length)
    except:
        post_data = 'Error reading POST data'

# Output response
print("Content-Type: text/html")
print()  # Blank line separates headers from body
print("<html>")
print("<head><title>POST Test</title></head>")
print("<body>")
print("<h1>CGI POST Test Results</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
print(f"<li>REQUEST_METHOD: {method}</li>")
print(f"<li>CONTENT_TYPE: {content_type}</li>")
print(f"<li>CONTENT_LENGTH: {content_length}</li>")
print(f"<li>QUERY_STRING: {query_string}</li>")
print("</ul>")
print("<h2>POST Data Received:</h2>")
print(f"<pre>{post_data}</pre>")
print("</body>")
print("</html>")