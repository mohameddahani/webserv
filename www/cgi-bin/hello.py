#!/usr/bin/env python3
from datetime import datetime

currentDateTime = datetime.now()

print("Content-Type: text/html\n")
print()

print ("<html>")
print ("<head><title>Python CGI Test</title></head>")
print ("<body>")
print ("<h1>Hello from Python CGI! </h1>")
print ("<p>If you see this, Python CGI is working!</p>")
print (f"<p>Current time: #{currentDateTime}</p>")
print ("</body>")
print ("</html>")
