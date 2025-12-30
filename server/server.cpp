/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 20:44:41 by mait-all          #+#    #+#             */
/*   Updated: 2025/12/30 11:50:08 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"


// Default constructor
Server::Server()
{
	_sockfd = -1;
}

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

void	Server::run() {
	Request				req;
	Response			res;
	int					sockfd;
	int					connSock;
	int					epollfd;
	int					nfds;
	int					opt;
	struct sockaddr_in	address;
	socklen_t			addrlen;
	struct epoll_event	ev, events[MAX_EVENTS];
	const char 				*hello="HTTP/1.1 200 OK\r\n"
	                    			"Content-Type: text/plain\r\n"
	                   			 	"Content-Length: 11\r\n"
	                   				 "\r\n"
	                   				 "Sir, Tal3b!";

	// Initialization 
	address.sin_family = IPv4;
	address.sin_addr.s_addr = htonl(IP);
	address.sin_port = htons(PORT);
	std::memset(address.sin_zero, 0, sizeof(address.sin_zero));
	opt = 1;
	addrlen = sizeof(address);

	// Socket Creation
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		throwError("socket()");
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throwError("setsocketopt()");
	
	// Socket Identification
	if (bind(sockfd, (struct sockaddr *)&address, addrlen) < 0)
		throwError("bind()");

	// Listen for incoming connections
	if (listen(sockfd, BACK_LOG) < 0)
		throwError("listen()");
	
	epollfd = epoll_create(1024);
	if (epollfd < 0)
		throwError("epoll_create()");
	
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) < 0)
		throwError("epoll_ctl(sockfd)");
	
	bool	running = true;
	while (running)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds < 0)
			throwError("epoll_wait()");
		
		for (int i = 0; i < nfds ; i++)
		{
			if (events[i].data.fd == sockfd)
			{
				connSock = accept(sockfd, (struct sockaddr *)&address, &addrlen);
				if (connSock < 0)
					throwError("accept()");
				fcntl(connSock, F_SETFL, O_NONBLOCK);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connSock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connSock, &ev) < 0)
					throwError("epoll_ctl(connSock)");
			}
			else
			{
				char	buffer[MAX_BUFFER_SIZE];
				size_t	bytesReaded;
				bytesReaded = recv(connSock, buffer, MAX_BUFFER_SIZE - 1, 0);
				if (bytesReaded < 0)
					throwError("recv()");
				buffer[bytesReaded] = '\0';
				std::cout << "=== Request received ===\n";
				std::cout << buffer << std::endl;
				
				////////////////////////////////////////////////////////////////////
				// Pass received request to Request parser (Lahya is on line)
				////////////////////////////////////////////////////////////////////
				
				////////////////////////////////////////////////////////////////////
				// Then building the response (Al Dahmani is for it)
				////////////////////////////////////////////////////////////////////

				// just for testing a hello response send to each client
				send(events[i].data.fd, hello, std::strlen(hello), 0);
				
				close(events[i].data.fd);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
			}
		}
		
	}
	
	close(sockfd);
	close(epollfd);
		
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
