## Implementation of a basic http server
To implement a basic http server we need to learn first some basic important stuff.
### 1. Implementing a TCP connection
To implement TCP, we have to learn about **TCP sockets**.
#### What is a socket?
A **socket** is the mechanism that most popular operating systems provide to give programs access to the network. It **allows messages to be sent and received between applications (unrelated processes)** on different networked machines.
<br>

It's also an endpoint of a two way communication link between two programs running on the network.
#### How sockets work in network?
the socket provides bedirectional **FIFO** communication facility over the network. we use `socket` system call to create a socket.
- A socket connecting to the network is created at each end of the communication.
- Each socket has a specific address.
- This address is composed of an IP address and a port number.
- Socket are generally employed in client server applications.
- The server creates a socket, attaches it to a network port addresses then waits for the client to contact it.
- The client creates a socket and then attempts to connect to the server socket.
When the connection is established, transfer of data takes place.

#### Types of Sockets
- **Datagram Sockets (UDP)**: This is a type of network which has connection less point for sending and receiving packets. it's similar to mainbox, the data posted into the box are collected and delivered to a letterbox (receviving socket).

- **Stream Sockets (TCP)**: it's a type of ***interprocess communications*** socket or network socket which provides a connection-oriented, sequenced, and unique flow of data. it's similar to phones, when the connection established (two ends), and the transfer of data takes place (the conversation).

#### Function calls in Socket Programming
- **`Socket()`**: to create a socket
- **`Bind()`**: identify socket
- **`Listen()`**: Ready to receive a connection
- **`Connect()`**: Ready to act as a sender
- **`Accept()`**: Confirmation
- **`Write()`**: To send data
- **`Read()`**: To receive data
- **`Close()`**: To close a connection

#### Programming with TCP/IP sockets
There are a few steps involved in using sockets:

1. Create the socket
2. Identify the socket
3. On the server, wait for an incoming connection
4. Send and receive messages
5. Close the socket

##### 1. Creating a socket
**Syntax:**

```cpp
int socket(int domain, int type, int protocol);
```

**Example:**

```cpp
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
```
- AF_INET: indicates Ipv4 protocol.
- SOCK_STREAM: indicates a TCP socket
- 0: lets the system choose the default protocol for the specified address family and socket type (which is TCP in this case).

##### 2. Identifying the socket
In this phase we're going to set a transport address to the socket (a port number in IP networking). and we can do that using `bind()` system call.
This the phase where we set an IP and a port number to our socket, to allow it listen for incomming connections.
<br>

**Syntax:**
```cpp
int	bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

- `sockfd`: the file descriptor created with socket() system call.
- `sockaddr`: is a generic container that allows the OS to be able to read the first couple of bytes that identify the address family. For IP networking, we use **struct sockaddr_in**, which is defined in header **'netinet/in.h'**, This structure defines:
```cpp
struct sockaddr_in 
{ 
    __uint8_t         sin_len; 
    sa_family_t       sin_family; 
    in_port_t         sin_port; 
    struct in_addr    sin_addr; 
    char              sin_zero[8]; 
};
```

We need to fill out this struct before calling *bind*, the three key parts we need to fill are:
<br>

-> **sin_family** which is the address family we used to set up the socket, in our case 'AF_INET'.
<br>

-> **sin_port** the port number where you'll receive a connection.
<br>

-> **sin_addr**  the IP address.

- `addrlen`: the length of the sockaddr used, in our case **'sizeof(struct socaddr_in)'**.


**Example of Binding a socket:**
```cpp
#include <sys/socket.h> 
… 
struct sockaddr_in address;
const int PORT = 8080;
/* htonl converts a long integer (e.g. address) to a network representation */ 
/* htons converts a short integer (e.g. port) to a network representation */ 
memset((char *)&address, 0, sizeof(address)); 
address.sin_family = AF_INET; 
address.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY: accepts connections on any IP (default 0.0.0.0)
address.sin_port = htons(PORT); 
if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) 
{ 
    perror(“bind failed”); 
    return 0; 
}
```

##### 3. On the server, wait for an incomming connection
the `listen()` system call does this job of listening for incomming connections
**Syntax:**
```cpp
int	listen(int sockfd, int backlog);
```

- `sockfd`: file descriptor created by socket syscall.
- `backlog`: defines the maximum number of pending connections that can be queued up before connections are refused.

**Example:**
```cpp
#include <sys/socket.h>

listen(serverSocket, 5);
```

Then we need to grabs the first connection request on the queue of pending connections, and creates a new socket for that connection, this does using `accept()` system call.

> **Note**: The original socket that was set up for listening is used only for accepting connections, not for exchanging data. By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.

**Syntax:**
```cpp
#include <sys/socket.h>

int	accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
- **`sockfd`**: socket that was set up for accepting connections with `listen()`.
- **`addr`**: is the addresss structure that gets filled in with the address of the client that is doing the connect. This allows us to examine the IP and port number of the connecting socket if we want to.
- **`addrlen`**: the length of the address structure (pointed by `addr`).

**Example of listening and accepting a socket:**
```cpp
if (listen(server_fd, 3) < 0) 
{ 
    perror(“In listen”); 
    exit(EXIT_FAILURE); 
}
if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
{
    perror("In accept");            
    exit(EXIT_FAILURE);        
}
```

##### 4. Send and receive messages
At this point, We finally have connected sockets between a client and a server.
<br>

The same `read` and `write` system calls that work on files also work on sockets.

**Example:**
```cpp
char buffer[1024] = {0};
int valread = read( new_socket , buffer, 1024); 
printf(“%s\n”,buffer );
if(valread < 0)
{ 
    printf("No bytes are there to read");
}
char *hello = "Hello from the server";//IMPORTANT! WE WILL GET TO IT
write(new_socket , hello , strlen(hello));
```

> **Note:** the real working of HTTP server happens based on the content present in `char*	hello` var.

##### 5. Close the socket
The final phase is closing the communication, by using `close()` syscall.
**Syntax:**

```cpp
close(new_socket);
```

#### Full Example of a server-side socket
```cpp
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080
int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    char *hello = "Hello from server";
    
    // step 1: Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    
	// step 2: Binding or identifying the created socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
	
	// step 3: waiting for connections
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

		char	*client_ip = inet_ntoa(address.sin_addr);
		int		client_port = ntohs(address.sin_port);

		printf("\n++++++ Connection accepted from %s:%d\n", client_ip, client_port);
        
		// step 4: send and receive data
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------\n");

		// step 5: closing connection
        close(new_socket);
    }
    return 0;
}
```
### 2. Implement a basic http server
First of all we'll take a look at the interaction between server and web browser.
#### HTTP
This the basic interaction between a browser and a server:
<br>


```bash

 _____________   1. Sends HTTP Request Msg          _____________
|             | ------------------------------->   |             | 
| HTTP Client |                                    | HTTP Server |
|_____________| <-------------------------------   |_____________|
                 2. Sends HTTP Response Msg
    Browser                                            Server


```

Now, let's a look at client-side and what they send and what they receive.

#### HTTP Client (Web Browser):
When we want to connect to the server, the usually process is by typing some URL/Address of a website in the browser and then press enter, for example:
<br>
**`http://www.example.com:80/index.html`**

To display the page, the browser fetches the file `index.html` from the server.
<br>

If the server is configured to certain default pages, it's displayed first when you visit the server, like when you visit a folder on the server.
<br>

Some servers have `public.html` or `index.html` as default pages.
<br>

In our case, we consider `index.html` as default page.

**Send HTTP Request - Write lines to socket**
```bash
  Method    URL     Protocol Version
 	 |       |          |          ____
	Get /index.html HTTP/1.1           |
	Host: www.example.com              |
	User-Agent: Chrome/142.0.0.0       |
	Accept: text/html, */*             |-------> Request Header
	Accept-Language: ar-ma             |
	Accept-Charset: ISO-8859-1,utf-8   |
	Connection: keep-alive             |
                                   ____|
                                   ____
	--- Blank line ----                |
	For POST and PUT                   |
	       Methods                     |-------> Request Body
                                   ____|          (Optional)
```

--- 

**What happens?**
What would happen if we run our `server.c` example and tap `http://localhost:8080/index.html` in our web browser.
<br>

We'll get the similar output as shown in the Request structure above, with some small differences. But we'll get an issue at the browser that is **This page isn't working...**.
<br>

So, what is the problem? and Why can't we see the data that we have send from the server?
<br>

The answer will in the next section.

#### HTTP Server:

The point is about the variable `char *hello` in **step 4: send and receive data**.
<br>

The problem appears, because the client (Browser) doesn't understand the response we send in the variable `char *hello`, and it's time to make things clear and send to the client what they want!.
<br>

The client sent us some headers (in `HTTP Request`) and expects same from us in return. an `HTTP Response` header.
<br>

**Send HTTP Response - Read lines from socket**

```bash
   Version   Status code  Status Msg 
 	  |           |        |           ____
	HTTP/1.1     200       OK              |
	Date: Sun, 06 Dec 2025 09:45:21 GMT    |
	Server: *server_name*                  |
	Content-Type: text/html;charset-UTF-8  |-------> Response Header
	Content-Length: 1846                   |
                                       ____|
                                       ____
	--- Blank line ----                    |
	<?xml ....>                            |
	<!DOCTYPE html ....>                   |
	<html ....>                            |-------> Response Body
	   ....                                |
	</html>                                |
                                       ____|          
```

So, if we want to send `Hello from server`, We need first to construct the Header Then insert a `blank line`, then we can send our msg/data.
<br>

The headers shown above are just an example. In fact there are many Headers present in HTTP, Look at HTTP RFCS from **`RFC 7230`** to **`RFC 7235`**
<br>

Now, to make our server work, we'll just construct a minimal HTTP header.
```cpp
char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
```

Consider to calculate how many bytes (`Content-Length`) you're trying to send in Body, and also the type of data you're sending (`Content-Type`).



## Blocking and Non Blocking socket Concepts

### Blocking
By default, all socket operations are blocking, `accept()`, `read() / recv()`, `write() / send()`, are all blocking functions, which means your program will stop or will wait untill that operation finish, for example `accept()` will sleep untill a new connection comes and `read()` or `recv()` also will wait untill some data found on the file descriptor.
<br>
This will cause a problem that is we'll not be able to handle multiple clients at the same time, because if your server get's stuck and waiting for a client to send him data, your body will not be able to handle new comming connections.
<br>
There some traditinal solutions to handle that problem like using `Multi-threading` or `Multi-processing( fork() )`, but these ways are complex, and cost intensive resources.

### Non Blocking

Non-blocking will never halt our program, if an operation cannot be completed immediately, It will return an error (`EAGAIN` or `EWOULDBLOCK`) instead of waiting.

- **`accept()`**: If no clients are waiting, it returns immediately with an error.
- **`read()`**: If there's no data to read, it returns immediately with an error.
- **`write()`**: If the kernel's send buffer is full, it returns immediately with an error.

##### How we can set a socket to Non-blocking?

By using the `fcntl()` (file control) system call function to manipulate our file descriptor.
```c
#include <fcntl.h>
#include <unistd.h>

int set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        return -1;
    }
    return 0;
}
```

> **Note**: if we set a socket to non blocking and we try to read from it in a loop `while (true)`, we'll spin the CPU at 100% usage. and we will get `EAGAIN` imediately, this called a **busy-wait** problem.

This where **I/O Multiplexing** comes in.

## I/O Multiplexing
**I/O Multiplexing** allows us to manipulate multiple file descriptors (sockets) at the same time and get notified when one of them is ready fo an I/O operation.
<br>

`epoll` is the modern and efficient I/O multiplexing API on linux.
