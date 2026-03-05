# Configuration File Format

## Overview
Webserv uses an nginx-inspired configuration format. The configuration file defines how your server behaves, including which ports to listen on, where to serve files from, and how to handle different types of requests.

Configuration files are typically stored in the `config/` directory and passed to the server at startup:
```bash
./webserv config/default.conf
```

---


## Basic Structure

A configuration file consists of a `server` block. Which defines a server that listens on a specific or multiple ports.

```nginx
server {
    # Server-level directives
    listen 8080;
	host 127.0.0.1;
    root www;
    
    # Location blocks for specific paths
    location / {
        # Location-level directives
    }

	# Cgi configurations
	cgi_conf .py /usr/bin/python3;
}
```

---

## Server Block Directives

### `listen`
**Syntax:** `listen <port>;`  
**Context:** server  
**Required:** Yes

Specifies the port number the server listens on.
```nginx
listen 8080;
```

**Example:**
```nginx
server {
    listen 8080;  # Listen on port 8080
}
```

**Example Of Multiple Ports:**
```nginx
server {
    listen 8080;  # Listen on port 8080
    listen 8081;  # Listen on port 8081
    listen 1337;  # Listen on port 1337
}
```

---

### `host`
**Syntax:** `host <ip_address>;`  
**Context:** server  
**Default:** `127.0.0.1` (localhost)

Specifies the IP address to bind to.
```nginx
host 127.0.0.1;   # Listen only on localhost
host 127.10.15.3; # Listen only on 127.10.15.3 (consider that this IP is under reserved local IPs range)
```

---

### `server_name`
**Syntax:** `server_name <name>;`  
**Context:** server  
**Default:** `webserv/1.0`

Sets the server identification string sent in response headers.
```nginx
server_name webserv/1.0;
```

---

### `root`
**Syntax:** `root <path>;`  
**Context:** server, location  
**Default:** www folder

Defines the root directory for serving files.
```nginx
root www;
```

**How it works:**
- Request: `GET /images/photo.jpg`
- With `root www;`
- Server looks for: `www/images/photo.jpg`

---

### `client_max_body_size`
**Syntax:** `client_max_body_size <mega_bytes>;`  
**Context:** server  
**Default:** 3000000 MB

Limits the maximum size of the client request body (POST data).
```nginx
client_max_body_size 3;  # 3MB in bytes
client_max_body_size 10; # 10MB in bytes
```

**Note:** If a client sends a body larger than this, the server responds with `413 Payload Too Large`.

---

### `index`
**Syntax:** `index <file>;`  
**Context:** server, location  
**Default:** `index.html`

Defines the default file to serve when a directory is requested.
```nginx
index index.html;
```

**How it works:**
- Request: `GET /`
- Server looks for: `www/index.html`
- If found serve it, if Not return 404

---

### `error_page`
**Syntax:** `error_page <code> <path>;`  
**Context:** server

Defines custom error pages for specific HTTP status codes.
```nginx
error_page 404 errors/404.html;
error_page 500 errors/500.html;
error_page 403 errors/403.html;
```

**How it works:**
- When server returns 404, it serves the file at `www/errors/404.html`
- If custom error page doesn't exist, built-in default is used

**Supported error codes:**
- `400` - Bad Request
- `403` - Forbidden
- `404` - Not Found
- `405` - Method Not Allowed
- `413` - Payload Too Large
- `500` - Internal Server Error
- `503` - Service Unavailable
- `504` - Gateway Timeout

---

### `cgi_conf`
**Syntax:** `cgi_conf <extension> <interpreter_path>;`  
**Context:** server  
**Default:** bash & python

Configures CGI script execution for files with specific extensions.
```nginx
cgi_conf .py /usr/bin/python3;
cgi_conf .php /usr/bin/php-cgi;
cgi_conf .rb /usr/bin/ruby;
cgi_conf .sh /bin/bash;
```

**How it works:**
- Request: `GET /cgi-bin/script.py`
- Server executes: `/usr/bin/python3 ./cgi-bin/script.py`
- Output is sent to client

**Requirements:**
- Scripts must be executable (`chmod +x script.py`)
- Interpreter path must be valid

---

## Location Block

Location blocks define specific behavior for requests matching a path pattern.

### Basic Syntax
```nginx
location <path> {
    # Directives that apply to this path
}
```

### `allow_methods`
**Syntax:** `allow_methods <method1> [method2] [method3];`  
**Context:** location  

Restricts which HTTP methods are allowed for this location.
```nginx
location / {
    allow_methods GET;  # Only GET allowed
}

location /upload {
    allow_methods POST;  # Only POST allowed
}

location /api {
    allow_methods GET POST DELETE;  # Multiple methods
}
```

**Supported methods:** `GET`, `POST`, `DELETE`

**Note:** Requests with disallowed methods receive `405 Method Not Allowed`.

---

### `autoindex`
**Syntax:** `autoindex on | off;`  
**Context:** location

Enables automatic directory listing when no index file is found.
```nginx
location /downloads/ {
    autoindex on;  # Show directory contents
}
```

**Example output:**
```
📂 Index of /downloads


Name               Type         Size

file1.txt          File         1.2KB
file2.pdf          File         453KB
subdirectory/      Directory      —
```

---

### `return`
**Syntax:** `return <url>;`  
**Context:** location

Returns an HTTP redirect response.
```nginx
location /old-page {
    return /new-page; # redirect to new-page
}

location /uploads {
	return /sign-in; # redirect to sign-in
}

```

---

## Complete Example
```nginx
server {
    # Server configuration
    listen 8080;
    host 127.0.0.1;
    server_name webserv/1.0;
    root www;
    client_max_body_size 30; # 30MB
    index index.html;
    
    # Custom error pages
    error_page 400 errors/400.html;
    error_page 403 errors/403.html;
    error_page 404 errors/404.html;
    error_page 405 errors/405.html;
    error_page 413 errors/413.html;
    error_page 500 errors/500.html;
    error_page 503 errors/503.html;
    error_page 504 errors/504.html;
    
    
    # Root location - GET only
    location / {
        allowed_methods GET;
        root www;
        index index.html;
    }
    
    # Uploads with directory listing
    location /uploads {
        allowed_methods GET;
        autoindex on;
    }
    
    # Redirects
    location /old-url {
        return /new-url;
    }

    # CGI scripts
    location /cgi-bin/ {
        allowed_methods GET POST;
        root www;
    }

    # CGI configuration
    cgi_conf .py /usr/bin/python3;
    cgi_conf .php /usr/bin/php-cgi;
    cgi_conf .rb /usr/bin/ruby;
    cgi_conf .sh /bin/bash;
}
```

---

## Troubleshooting

### Server won't start

**Problem:** Port already in use
```
Error: bind() failed: Address already in use
```

**Solution:** Change the port or stop the other process
```nginx
listen 8081;  # Try a different port
```

---

### 404 Not Found

**Problem:** File not found

**Check:**
1. Is `root` path correct?
2. Does the file exist at `root/requested/path`?
3. Do you have read permissions?
```nginx
server {
    root www;  # Check this path
}
```

---

### 413 Payload Too Large

**Problem:** Upload too large

**Solution:** Increase `client_max_body_size`
```nginx
server {
    client_max_body_size 100;  # 100MB
}
```

---

### 405 Method Not Allowed

**Problem:** Method not allowed for this location

**Solution:** Add method to `allowed_methods`
```nginx
location / {
    allowed_methods GET POST;  # Add POST
}
```

---

### CGI Script Not Executing

**Problem:** CGI returns 500 error

**Check:**
1. Is interpreter path correct? (`which python3`)
2. Is script executable? (`chmod +x script.py`)
3. Is script has a valid syntax?

---


## See Also
- [API Reference](API.md) - HTTP API documentation
- [Webserv Architecture](DESIGN.md) - Webserv Architecture
