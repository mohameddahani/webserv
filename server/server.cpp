/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 13:05:03 by mdahani           #+#    #+#             */
/*   Updated: 2026/01/07 15:27:09 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// Default constructor
Server::Server()
{
	_sockfd = -1;
}

Server::Server(std::string& host, int port)
 : _sockfd(-1), _epollfd(-1), _port(port), _host(host)
{
	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
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

void	Server::initServerAddress()
{
	_serverAddr.sin_family = IPv4;
	_serverAddr.sin_addr.s_addr = inet_addr(_host.c_str());
	_serverAddr.sin_port = htons(_port);
	std::memset(_serverAddr.sin_zero, 0, sizeof(_serverAddr.sin_zero));
}

void	Server::createServerSocket()
{
	int	opt = 1;

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
		throwError("socket()");

	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throwError("setsockopt()");
	
	setNonBlocking(_sockfd);
}

void	Server::bindServerSocket()
{
	if (bind(_sockfd, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
		throwError("bind()");
}

void	Server::startListening()
{
	if (listen(_sockfd, BACK_LOG) < 0)
		throwError("bind()");
}

void	Server::createEpollInstance()
{
	_epollfd = epoll_create(1024);
	if (_epollfd < 0)
		throwError("epoll_create()");
}

void	Server::addServerToEpoll()
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _sockfd;
	
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _sockfd, &ev) < 0)
		throwError("epoll_ctl(_sockfd)");
}

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

void	Server::setUpNewConnection(int epfd, int serverFd, epoll_event& ev)
{
	int	clientFd;

	clientFd = accept(serverFd, NULL, NULL);
	if (clientFd < 0)
		return ;

	setNonBlocking(clientFd);

	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &ev) < 0)
		throwError("epoll_ctl(clientFd)"); // close fds if an error occurs
	t_clientState	clientState;
	clientState.fd = clientFd;
	clients[clientFd] = clientState;

	std::cout << "\n✅ New connection (fd: " << clientFd << ")\n" << std::endl;
}

bool	Server::recvRequest(int epfd, int notifiedFd, epoll_event ev)
{
	char	buffer[MAX_BUFFER_SIZE];
	size_t	bytesRead;
	bytesRead = recv(notifiedFd, buffer, MAX_BUFFER_SIZE - 1, 0);
	if ((int)bytesRead <= 0)
		{
			close(notifiedFd);
			clients.erase(notifiedFd);
			return (false);
		}
	clients[notifiedFd].request.append(buffer, bytesRead);
	clients[notifiedFd].bytes_received += bytesRead;
	size_t headerEnd = clients[notifiedFd].request.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return (false);

	if (isCompleteRequest(clients[notifiedFd].request) && !clients[notifiedFd].isPostRequest)
	{
		clients[notifiedFd].content_length = getContentLength(clients[notifiedFd].request);
		clients[notifiedFd].isPostRequest = true;
	}
	if (clients[notifiedFd].isPostRequest && (clients[notifiedFd].content_length > clients[notifiedFd].request.length() - headerEnd - 4))
		return (false);

	std::cout << "=== Request received ===\n";
	std::cout << clients[notifiedFd].request << std::endl;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = notifiedFd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, notifiedFd, &ev) < 0)
		throwError("epoll_ctl(client_fd)");

	return (true);
}

bool	Server::sendResponse(int epfd, int notifiedFd, Request &request)
{
	Response	res;

	if (!clients[notifiedFd].isHeaderSent)
	{
		res.response(request);
		std::string	responseHeaders = res.getHeaders();
		size_t		headersLength = responseHeaders.length();
		ssize_t		bytesSent;

		bytesSent = send(notifiedFd, responseHeaders.c_str(), headersLength, 0);
		if (bytesSent < 0)
			throwError("send() when sending header part");
		clients[notifiedFd].isHeaderSent = true;
	}
	if (clients[notifiedFd].isHeaderSent)
	{
		char	buffer[MAX_BUFFER_SIZE];
		ssize_t	bytesRead;
		ssize_t	bytesSent;
		bytesRead = read(res.getBodyFd(), buffer, sizeof(buffer));
		if (bytesRead <= 0)
		{
			clients.erase(notifiedFd);
			epoll_ctl(epfd, EPOLL_CTL_DEL, notifiedFd, NULL);
			close(notifiedFd);
			close(res.getBodyFd());
			return (true);
		}
		bytesSent = send(notifiedFd, buffer, bytesRead, 0);
		if (bytesSent < 0)
		{
				close(notifiedFd);
				clients.erase(notifiedFd);
				return (false);
		}
		std::memset(buffer, '\0', sizeof(buffer));
	}
	return (false);
}

void	Server::processServerEvent(struct epoll_event& ev)
{
	setUpNewConnection(_epollfd, _sockfd, ev);
}

void	Server::processClientEvent(struct epoll_event& event, Request& req)
{
	int fd = event.data.fd;
	
	if (event.events & EPOLLIN) // Read event => Request received
	{
		struct epoll_event ev;
		if (!recvRequest(_epollfd, fd, ev))
			return ;
		req.setRequest(clients[fd].request);
	}
	else if (event.events & EPOLLOUT) // Write event => Send response
	{
		if(!sendResponse(_epollfd, fd, req))
			return ;
	}
	else // Error event => EPOLLERR
	{
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
		close (fd);
		return ;
	}
}

void	Server::handleEpollEvents(int nfds, struct epoll_event* events, Request& req)
{
	for (int i = 0; i < nfds; i++)
	{
		if (events[i].data.fd == _sockfd)
			processServerEvent(events[i]);
		else
			processClientEvent(events[i], req);
	}
}

void	Server::run(Request &req) {
	int	n_fds;

	// init server address
	initServerAddress();

	// socket creation
	createServerSocket();

	// socket identification
	bindServerSocket();

	// socket listening
	startListening();

	// epoll's instance creation
	createEpollInstance();

	// setup server socket to accept new connections
	addServerToEpoll();

	// running server and waiting for connections
	struct epoll_event events[MAX_EVENTS];

	std::cout << "🚀 Server running on " << _host << ":" << _port << std::endl;

	bool	running = true;
	while (running)
	{
		n_fds = epoll_wait(_epollfd, events, MAX_EVENTS, -1);
		if (n_fds < 0)
			throwError("epoll_wait()");
		handleEpollEvents(n_fds, events, req);
	}

	close(_sockfd);
	close(_epollfd);
}
