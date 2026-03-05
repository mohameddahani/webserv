# Webserv Architecture & Design

## Overview

Webserv is a high-performance HTTP/1.0 web server written in C++98. It uses an event-driven, non-blocking I/O architecture inspired by nginx to handle thousands of concurrent connections efficiently.

### Key Design Goals

1. **Performance** - Handle multiple concurrent connections without threads
2. **Compliance** - Strict C++98 standard, no external dependencies
3. **Reliability** - Graceful error handling, no memory leaks
4. **Simplicity** - Clean code architecture, easy to understand

---

## System Architecture

### High-Level Overview

<br>
<img src="../assets/high-level-overview.png" alt="High Level Overview">
<br>

---

## Core Components

### 1. Server Core

**File:** `src/server/Server.cpp`

**Responsibilities:**
- Main event loop using epoll
- Orchestrates all other components
- Manages client connections
- Handles timeouts

**Key Methods:**
```cpp
void run();                    // Main event loop
void processServerEvent();     // Handle new connections
void processClientEvent();     // Handle client I/O
void checkClientTimeOut();     // Remove stale connections
```

**Event Loop:**

<br>
<img src="../assets/event-loop.png" alt="Event Loop">
<br>

---


### 2. Listener (Socket Manager)

**File:** `src/server/Listener.cpp`

**Responsibilities:**
- Create and bind server sockets
- Set socket options (SO_REUSEADDR, non-blocking)
- Listen for connections
- Track server file descriptors

**Socket Setup:**
```cpp
1. socket()              // Create socket
2. setsockopt()          // SO_REUSEADDR
3. bind()                // Bind to port
4. listen()              // Start listening
5. fcntl(O_NONBLOCK)     // Non-blocking mode
```

---

### 3. Epoll (I/O Multiplexer)

**File:** `src/server/Epoll.cpp`

**Responsibilities:**
- Monitor multiple file descriptors
- Notify when I/O is ready
- Efficient event-driven architecture

**Key Operations:**
```cpp
epoll_create()           // Create epoll instance
epoll_ctl(ADD)           // Add fd to monitor
epoll_ctl(MOD)           // Modify events
epoll_ctl(DEL)           // Remove fd
epoll_wait()             // Wait for events
```

**Why Epoll?**
- ✅ O(1) performance (vs O(n) for select/poll)
- ✅ Handles 10,000+ connections
- ✅ Edge-triggered and level-triggered modes
- ✅ Linux standard for high-performance servers

---

### 4. Connection Manager

**File:** `src/server/ConnectionManager.cpp`

**Responsibilities:**
- Accept new connections
- Read data from clients (recv)
- Write data to clients (send)
- Close connections
- Handle partial I/O

**Connection Lifecycle:**
<br>
<img src="../assets/client-connection-lifecycle.svg" alt="Connection Lifecycle">
<br>

---


### 5. Client State Machine

**File:** `src/Client.cpp`

**Responsibilities:**
- Store per-client state
- Buffer incomplete requests
- Track CGI execution
- Manage response state

**Client Data:**
```cpp
class Client {
    std::string _request;          // Buffered request
	size_t		_bytesReceived;    // Track bytes received
	size_t		_contentLength;    // Get content length if it's post request
	time_t		_lastActivity;     // For timeouts
	bool		_isPostRequest;    // Is it Post Request?
    bool        _requestComplete;  // Request fully received?
	bool		_isHeaderSent;     // Does header part sent?
	bool		_isTimedOut;       // Does client timed out?
	int			_clientFd;         // Client socket fd
	int			_bodyFd;           // Body fd

    int         _cgiPipeFd;        // CGI output pipe
    pid_t       _cgiPid;           // CGI process ID
    bool        _isCgiRunning;     // CGI in progress?
    bool        _isCgiTimedOut;    // Does CGI timed out?
    std::string	_cgiOutput;        // Buffered CGI output
	time_t		_cgiStartTime;     // For CGI timeouts
	size_t		_cgiBytesSent;     // Tracking CGI bytes sent
};
```

---

### 6. HTTP Request Parser

**File:** `src/http/Request.cpp`

**Responsibilities:**
- Parse HTTP request line
- Parse headers
- Extract body
- Validate syntax

---

### 7. HTTP Response Builder

**File:** `src/http/Response.cpp`

**Responsibilities:**
- Build HTTP response
- Set status codes
- Add headers
- Handle static files

---

### 8. CGI Handler

**File:** `src/cgi/CgiHandler.cpp`

**Responsibilities:**
- Fork child process
- Setup environment variables
- Create pipes for I/O
- Execute CGI script
- Read output

**CGI Execution Flow:**
<br>
<img src="../assets/cgi-workflow.svg" alt="CGI Workflow">
<br>


**Non-Blocking CGI:**
```
Why non-blocking?
- CGI script might take seconds to run
- Can't block the entire server
- Need to serve other clients meanwhile

Solution:
1. Fork CGI, add pipe to epoll
2. Continue serving other clients
3. When pipe has data, read it
4. When complete, send response
```

---

## Data Flow


### Complete Request/Response Cycle

<br>
<img src="../assets/complete-lifecycle.svg" alt="Complete Request/Response Cycle">
<br>

---

## Key Design Decisions

### 1. Single-Threaded vs Multi-Threaded

**Decision:** Single-threaded event loop

**Rationale:**
- I/O-bound workload (not CPU-bound)
- No synchronization overhead
- Simpler code, fewer bugs
- Scales well with epoll
- Nginx uses same approach

---

### 2. epoll vs select/poll

**Decision:** Use epoll

**Rationale:**
- O(1) performance
- No fd_set size limits
- Efficient for 1000+ connections
- Linux standard

---

### 3. Non-Blocking I/O

**Decision:** All sockets are non-blocking

**Rationale:**
- Prevents one slow client blocking others
- Predictable latency
- Enables concurrent handling

---

### 5. Configuration Format

**Decision:** nginx-inspired syntax

**Rationale:**
- Familiar to sysadmins
- Hierarchical (server blocks, locations)
- Human-readable

---

## Security Architecture

### Input Validation

- Validate HTTP method
- Check URI for path traversal (`..`)
- Enforce body size limits
- Sanitize headers

### Resource Limits

- Maximum connections
- Request timeout
- CGI execution timeout
- Upload size limits

### File Access Control

- Restrict to document root
- Check file permissions

---


## Error Handling Philosophy

1. **Never crash** - All errors caught and handled
2. **Fail gracefully** - Return error page, don't hang
3. **Log errors** - Help debugging
4. **Clean up** - No resource leaks

**Error categories:**
- Socket errors → close connection
- Parse errors → 400 Bad Request
- File errors → 404/403
- CGI errors → 500/504

---


## Testing Approach

The system is designed to be testable:

- **Unit testable:** Each component has clear interface
- **Integration testable:** Can test request/response cycle
- **Load testable:** Can measure performance under load

---

## Performance Design

The architecture prioritizes:

1. **Low latency** - Non-blocking I/O, efficient event loop
2. **High throughput** - Can serve many clients concurrently
3. **Resource efficiency** - Single thread, minimal memory

---

## References

- [The C10K Problem](http://www.kegel.com/c10k.html) - Scalability challenges
- [nginx Architecture](https://www.aosabook.org/en/nginx.html) - Design inspiration
- [RFC 1945](https://datatracker.ietf.org/doc/html/rfc1945) - HTTP/1.0 Specification
- [RFC 3875](https://tools.ietf.org/html/rfc3875) - CGI/1.1 Specification
- [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html) - Linux epoll API

---

## See Also

- [API Reference](API.md) - HTTP API documentation
- [Configuration Guide](CONFIG.md) - How to configure the server