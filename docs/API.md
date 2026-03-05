# HTTP API Reference

## Overview

Webserv implements HTTP/1.0 protocol with support for GET, POST, and DELETE methods. This document describes the server's behavior and how to interact with it.

---

## Base URL
```
http://localhost:8080
```

Replace `8080` with your configured port.

---

## HTTP Methods

### GET - Retrieve Resources

Retrieves files, directory listings, or executes CGI scripts.

**Request:**
```http
GET /path/to/resource HTTP/1.1
Host: localhost:8080
```

**Response:**
```http
HTTP/1.1 200 OK
Server: webserv/1.0
Content-Type: text/html
Content-Length: 1234

...
```

**Examples:**
```bash
# Get homepage
curl http://localhost:8080/

# Get specific file
curl http://localhost:8080/images/photo.jpg

# Execute CGI script
curl http://localhost:8080/cgi-bin/script.py?name=John
```

---

### POST - Submit Data

Submits data to the server (form submissions, file uploads, API requests).

**Request:**
```http
POST /upload.html HTTP/1.1
Host: localhost:8080
Content-Type: application/pdf
Content-Length: 27

------WebKitFormBoundaryv9VZscQHlgNYFQwG
Content-Disposition: form-data; name="filename="; filename="example.pdf"
Content-Type: application/pdf

(binary data here)
------WebKitFormBoundaryv9VZscQHlgNYFQwG--
```

**Response:**
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 45

File Uploaded Successfully ✅
```

**Examples:**
```bash
# Form submission
curl -X POST http://localhost:8080/sign-in.html \
  -d "email=bob@gmail.com&password=secret&rememberme=on"

# File upload
curl -X POST http://localhost:8080/upload.html \
  -F "file=@document.pdf"

# CGI POST
curl -X POST http://localhost:8080/cgi-bin/test_post.py \
  -d "username=admin&password=a1d2m3i4n5"
```

---

### DELETE - Remove Resources

Deletes files from the server.

**Request:**
```http
DELETE /uploads/file.txt HTTP/1.1
Host: localhost:8080
```

**Response:**
```http
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 19

File deleted successfully
```

**Examples:**
```bash
# Delete file
curl -X DELETE http://localhost:8080/uploads/document.pdf
```

---

## Status Codes

### Success Codes (2xx)

| Code | Status | Description |
|------|--------|-------------|
| 200 | OK | Request successful |
| 201 | Created | Resource created successfully |
| 204 | No Content | Request successful, no content to return |

### Redirection Codes (3xx)

| Code | Status | Description |
|------|--------|-------------|
| 301 | Moved Permanently | Resource permanently moved to new URL |
| 302 | Found | Resource temporarily at different URL |

### Client Error Codes (4xx)

| Code | Status | Description |
|------|--------|-------------|
| 400 | Bad Request | Malformed request syntax |
| 403 | Forbidden | Access denied |
| 404 | Not Found | Resource doesn't exist |
| 405 | Method Not Allowed | HTTP method not allowed for this resource |
| 413 | Payload Too Large | Request body exceeds size limit |

### Server Error Codes (5xx)

| Code | Status | Description |
|------|--------|-------------|
| 500 | Internal Server Error | Unexpected server error |
| 503 | Service Unavailable | Server temporarily unavailable |
| 504 | Gateway Timeout | CGI script timeout |

---

## Request Headers

### Required Headers
```http
Host: localhost:8080
```

### Common Optional Headers
```http
User-Agent: curl/7.68.0              # Client identification
Accept: text/html,application/json   # Accepted content types
Content-Type: application/json       # Request body type (POST)
Content-Length: 123                  # Request body size (POST)
```

---

## Static Files

### Some Supported MIME Types

| Extension | MIME Type | Description |
|-----------|-----------|-------------|
| .html, .htm, .shtml | text/html | HTML documents |
| .css | text/css | Stylesheets |
| .js | text/javascript | JavaScript |
| .json | application/json | JSON data |
| .txt | text/plain | Plain text |
| .jpg, .jpeg | image/jpeg | JPEG images |
| .png | image/png | PNG images |
| .gif | image/gif | GIF images |
| .pdf | application/pdf | PDF documents |
| .mp4 | video/mp4 | MP4 videos |
| .mpg, .mpeg | video/mpeg | videos |
| .mp3 | audio/mpeg | audios |

### Example Requests
```bash
# HTML page
curl http://localhost:8080/index.html

# CSS file
curl http://localhost:8080/css/style.css

# Image
curl http://localhost:8080/images/logo.png

# Video
curl http://localhost:8080/videos/intro.mp4
```

---

## CGI (Common Gateway Interface)

### CGI Script Execution

Scripts in `/cgi-bin/` are executed by configured interpreters.

**Environment Variables Passed to CGI:**

***GET Method:***
```bash
REQUEST_METHOD=GET
QUERY_STRING=name=John&age=30
SCRIPT_NAME=/cgi-bin/script.py
PATH_INFO=/extra/path
SERVER_NAME=localhost
SERVER_PORT=8080
SERVER_PROTOCOL=HTTP/1.1
GATEWAY_INTERFACE=CGI/1.1
```

***POST Method:***
```bash
REQUEST_METHOD=POST
QUERY_STRING=name=John&age=30
CONTENT_TYPE=application/x-www-form-urlencoded
CONTENT_LENGTH=27
SCRIPT_NAME=/cgi-bin/script.py
PATH_INFO=/extra/path
SERVER_NAME=localhost
SERVER_PORT=8080
SERVER_PROTOCOL=HTTP/1.1
SERVER_SOFTWARE=webserv/1.0
GATEWAY_INTERFACE=CGI/1.1
```

**CGI Script Example (Python):**
```python
#!/usr/bin/env python3
import os
import sys

# Read environment
method = os.environ.get('REQUEST_METHOD')
query = os.environ.get('QUERY_STRING', '')

# Output headers
print("Content-Type: text/html")
print()  # Blank line required

# Output body
print("")
print(f"Hello from CGI")
print(f"Method: {method}")
print(f"Query: {query}")
print("")
```

**Test CGI:**
```bash
# GET request
curl http://localhost:8080/cgi-bin/script.py?name=John

# POST request
curl -X POST http://localhost:8080/cgi-bin/script.py \
  -d "username=john&email=john@example.com"
```

---

## Directory Listings

When `autoindex on` is configured, directory requests return HTML listings.

**Request:**
```bash
curl http://localhost:8080/uploads/
```

**Response:**
```html

📂 Index of /uploads


Name               Type         Size

file1.txt          File         1.2KB
file2.pdf          File         453KB
subfolder/         Directory      —

```

---

## Error Handling

### Error Response Format
```http
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: 152


404 Not Found

404 Not Found
webserv/1.0


```

### Custom Error Pages

Configure custom error pages in config file:
```nginx
error_page 404 errors/404.html;
error_page 500 errors/500.html;
```

---

## Performance & Limits

| Limit | Default | Configurable |
|-------|---------|--------------|
| Request body size | 3000000MB | Yes (`client_max_body_size`) |
| Concurrent connections | 1000+ | No (system limited) |
| Request timeout | 30s | No |
| CGI timeout | 20s | No |

---

## Testing Examples

### Basic Tests
```bash
# Test server is running
curl -I http://localhost:8080/

# Test 404 error
curl http://localhost:8080/nonexistent

# Test method not allowed
curl -X PUT http://localhost:8080/

```

### Stress Tests
```bash
# Using Apache Bench
ab -n 1000 -c 100 http://localhost:8080/

# Using siege
siege -c 100 -r 10 http://localhost:8080/
```


## See Also
- [Configuration Guide](CONFIG.md) - How to configure the server
- [Webserv Architecture](DESIGN.md) - Webserv Architecture