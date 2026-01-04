/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 13:05:03 by mdahani           #+#    #+#             */
/*   Updated: 2026/01/04 10:40:04 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// Default constructor
Server::Server() { _sockfd = -1; }

// getter
int	Server::getSockFd() const
{
	return (_sockfd);
}

// setter
void	Server::setSockFd(int fd)
{
	this->_sockfd = fd;
}

// * Methods

void	Server::setNonBlocking(int fd)
{
	int	flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throwError("fcntl(F_GETFL)");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throwError("fcntl(F_SETFL)");
}

bool	Server::isCompleteRequest(std::string& request)
{
	if (request.find("Content-Length:") != std::string::npos)
		return (true);
	return (false);
}

size_t	Server::getContentLength(std::string& request)
{
	size_t	pos = request.find("Content-Length:");
	if (pos == std::string::npos)
		return (0);
	size_t	start = pos + 15;
	size_t	end = request.find("\r\n", start);
	if (end == std::string::npos)
		return (0);

	std::string	lengthStr = request.substr(start, end - start);
	return (std::atoll(lengthStr.c_str()));
}

void	Server::run() {
	// static int count = 0;
	Request				req;
	Response			res;
	int					server_fd;
	int					client_fd;
	int					epoll_fd;
	int					n_fds;
	int					opt;
	struct sockaddr_in	server_addr;
	socklen_t			server_len;
	struct epoll_event	ev, events[MAX_EVENTS];
	// const char 				*hello="HTTP/1.1 200 OK\r\n"
	//                     			"Content-Type: text/plain\r\n"
	//                    			 	"Content-Length: 11\r\n"
	//                    				 "\r\n"
	//                    				 "Sir, Tal3b!";

	// Initialization 
	server_addr.sin_family = IPv4;
	server_addr.sin_addr.s_addr = htonl(IP);
	server_addr.sin_port = htons(PORT);
	std::memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
	opt = 1;
	server_len = sizeof(server_addr);

	// Socket Creation
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		throwError("socket()");
	
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throwError("setsocketopt()");

	// Set socketfd to Non-blocking mode
	setNonBlocking(server_fd);
	
	// Socket Identification
	if (bind(server_fd, (struct sockaddr *)&server_addr, server_len) < 0)
		throwError("bind()");

	// Listen for incoming connections
	if (listen(server_fd, BACK_LOG) < 0)
		throwError("listen()");
	
	epoll_fd = epoll_create(1024);
	if (epoll_fd < 0)
		throwError("epoll_create()");
	
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0)
		throwError("epoll_ctl(server_fd)");
	
	bool	running = true;
	while (running)
	{
		n_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (n_fds < 0)
			throwError("epoll_wait()");
		
		for (int i = 0; i < n_fds ; i++)
		{
			// case 1: new connection comes, we should accept it
			if (events[i].data.fd == server_fd)
			{
					struct sockaddr_in	client_addr;
					socklen_t			client_len = sizeof(client_addr);
					client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
					if (client_fd < 0)
						continue;
					setNonBlocking(client_fd);
					ev.events = EPOLLIN;
					ev.data.fd = client_fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
						throwError("epoll_ctl(client_fd)"); // close fds if an error occurs
					std::cout << "\n\nconnection accepted from file:" << client_fd << std::endl;
					t_clientState	clientState;
					clientState.fd = client_fd;
					clients[client_fd] = clientState;
					// clients[client_fd].bytes_received = 0;
					// clients[client_fd].content_length = 0;
					// clients[client_fd].headers_complete = false;
					// clients[client_fd].request_complete = false;
					// clients[client_fd].isHeaderSent = false;
			}
			// case 2: handle client events (read/write/error)
			else
			{
				client_fd = events[i].data.fd;
				
				// Read event => Request received
				if (events[i].events & EPOLLIN)
				{
					char	buffer[10];
					size_t	bytesRead;
					bytesRead = recv(client_fd, buffer, 10 - 1, 0);
					if ((int)bytesRead <= 0)
						{
							close(events[i].data.fd);
							clients.erase(events[i].data.fd);
							continue;
						}
					// if (bytesRead == 0)
					// 	continue;
					clients[client_fd].request.append(buffer, bytesRead);
					clients[client_fd].bytes_received += bytesRead;
					// std::cout << "bytes readed: " << bytesRead << "count: " << count << std::endl;
					// count++;
					size_t headerEnd = clients[client_fd].request.find("\r\n\r\n");
					if (headerEnd == std::string::npos)
						continue;
					
					if (isCompleteRequest(clients[client_fd].request) && !clients[client_fd].doesGetContentLength)
					{
						clients[client_fd].content_length = getContentLength(clients[client_fd].request);
						clients[client_fd].doesGetContentLength = true;
					}
					if (clients[client_fd].doesGetContentLength && (clients[client_fd].content_length > clients[client_fd].request.length() - headerEnd - 4))
						continue;
					std::cout << "=== Request received ===\n";
					std::cout << clients[client_fd].request << std::endl;

					ev.events = EPOLLIN | EPOLLOUT;
					ev.data.fd = client_fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) < 0)
						throwError("epoll_ctl(client_fd)");

					req.setRequest(clients[client_fd].request);
				}
				// Write event => Send response
				else if (events[i].events & EPOLLOUT)
				{
					if (!clients[client_fd].isHeaderSent)
					{
						res.response(req);
						std::string	responseHeaders = res.getHeaders();
						size_t		headersLength = responseHeaders.length();
						ssize_t		bytesSent;
				
						bytesSent = send(client_fd, responseHeaders.c_str(), headersLength, 0);
						if (bytesSent < 0)
							throwError("send() when sending header part");
						clients[client_fd].isHeaderSent = true;
					}
					if (clients[client_fd].isHeaderSent)
					{
						char	buffer[100];
						ssize_t	bytesRead;
						ssize_t	bytesSent;

						bytesRead = read(res.getBodyFd(), buffer, sizeof(buffer));
						if (bytesRead <= 0)
						{
							clients.erase(client_fd);
							epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
							close(client_fd);
							close(res.getBodyFd());
							continue;
						}
						// std::cout << "--------------------------------------\n";
						// std::cout << bytesRead;
						// std::cout << "--------------------------------------\n";
						// * i change strlen by bytesRead
						ssize_t bytesSent = send(client_fd, buffer, bytesRead, 0);
						if (bytesSent < 0)
						{
								close(client_fd);
								clients.erase(client_fd);
								continue;
						}
						std::memset(buffer, '\0', sizeof(buffer));
					}
				}
				// Error event => EPOLLERR is setted
				else
				{
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close (client_fd);
					continue;
				}
			}
		}
		
	}
	
	close(server_fd);
	close(epoll_fd);

// 	bool	running = true;
// 	while (running)
// 	{
// 		int newSocket;
// 		if ((newSocket = accept(sockfd, (struct sockaddr*)&address, &addrlen)) < 0)
// 			throwError("accept()");
		
// 		char	buffer[4096];
// 		size_t		bytesReaded;
// 		bytesReaded = recv(newSocket, buffer, sizeof(buffer) - 1, 0);
// 		if (bytesReaded < 0)
// 			throwError("recv()");
// 		buffer[bytesReaded] = '\0';
// 		std::cout << "=== Request received ===\n";
// 		std::cout << buffer << std::endl;

// 		std::cout << "--- send response to client ---\n";
// 		send(newSocket, hello, strlen(hello), 0);

// 		close (newSocket);
// 	}

	

//   // ! Monitor the client that connect to the server
//   // ? The epoll API monitoring multiple
//   // ? file descriptors to see if I/O is possible on any of them.

//   // * this creates a new epoll instance and returns a file descriptor referring
//   // * to that instance.
//   // ? EPOLL_CLOEXEC this mean The kernel automatically closes epfd during
//   // ? execve() ? If this process calls `execve()`, automatically close `epfd`.
//   int epfd = epoll_create1(EPOLL_CLOEXEC);
//   if (epfd < 0) {
//     std::cerr << "epoll failed" << std::endl;
//   }

//   // * epoll_event It is a structure defined by the Linux system, used to
//   // * describe an event that you want epoll to monitor or report to you.
//   struct epoll_event ev;
//   // * Event type that we need to watch (read, write, error...)
//   ev.events = EPOLLIN; // * is ready for EPOLLIN (Read) || EPOLLOUT (Write)
//   // * Event data (usually FD)
//   ev.data.fd = sockfd; // * that will epoll_ctl return if is ready for
//                        // * (Read or Write)

//   // * epoll_ctl is list of fds that we need to watch it on epoll by events
//   // * rules
//   if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
//     std::cerr << "epoll_ctl failed" << std::endl;
//   }

//   // * this array of struct used by epoll_wait() to store ready file descriptor
//   // * events for EPOLLIN (Read) || EPOLLOUT (Write)
//   struct epoll_event events[MAX_EVENTS];

//   // ! accepts a new connection from a client (for servers).

//   // ? buffer that we store to him the request
//   char buffer[MAX_BUFFER_SIZE];
//   while (true) {

//     // * epoll_wait fill the array events by ready fds and return the fds that's
//     // * ready for (Read) || (Write)
//     // ? -1 is arg for time out and when we use -1 that's mean we still
//     // ? wait untill get an fd
//     int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

//     if (n < 0) {
//       std::cerr << "epoll_wait failed" << std::endl;
//     }

//     // ! Recive request from browser
//     for (int i = 0; i < n; i++) {
//       // * check if we have fd of server
//       if (events[i].data.fd == sockfd) {
//         // * The accept() function accepts a new connection from a client (for
//         // * servers). It extracts the first connection request on the queue of
//         // * pending connections and creates a new socket for that connection.
//         int clientFd =
//             accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t *)&addrLen);
//         if (clientFd < 0) {
//           std::cerr << "accept failed" << std::endl;
//         }

//         // * add new sockfd of client to watch mode
//         struct epoll_event clientEv;
//         clientEv.events = EPOLLIN;
//         clientEv.data.fd = clientFd;

//         epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &clientEv);
//       } else {
//         if (events[i].events & EPOLLIN) { // * EPOLLIN o r EPOLLERR
//           int clientFd = events[i].data.fd;

//           // * The recv() function is a system call that is used to receive data
//           // * from a connected socket which allows the client or server to read
//           // * incoming messages. and is return the number of bytes that read
//           // ? 0 mean read data normaly ()
//           int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
//           buffer[bytesRead] = '\0';
//           if (bytesRead <= 0) {
//             epoll_ctl(epfd, EPOLL_CTL_DEL, clientFd, NULL);
//             close(clientFd);
//           }
//           std::cout << "==========Request==========" << std::endl;
//           std::string request = buffer;
//           std::cout << request << std::endl;
//           req.setRequest(request);
//           // * add new sockfd of client to watch mode again
//           struct epoll_event clientEv;
//           clientEv.events = EPOLLIN | EPOLLOUT;
//           clientEv.data.fd = clientFd;

//           epoll_ctl(epfd, EPOLL_CTL_MOD, clientFd, &clientEv);
//         }
//         if (events[i].events & EPOLLOUT) { // * EPOLLIN or EPOLLERR
//           std::string request = buffer;

//           // * Response
//           res.response(events[i].data.fd, req);

//           epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
//           close(events[i].data.fd);
//         }
//         if (events[i].data.fd & EPOLLERR) {
//           epoll_ctl(events[i].data.fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
//           close(events[i].data.fd);
//         }
//       }
//     }
//   }
}
