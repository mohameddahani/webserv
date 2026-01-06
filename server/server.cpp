/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 13:05:03 by mdahani           #+#    #+#             */
/*   Updated: 2026/01/06 09:29:17 by mait-all         ###   ########.fr       */
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

void	Server::setUpNewConnection(int epfd, int serverFd, epoll_event ev)
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

void	Server::sendResponse(int epfd, int notifiedFd, Request &request)
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
			return ;
		}
		bytesSent = send(notifiedFd, buffer, bytesRead, 0);
		if (bytesSent < 0)
		{
				close(notifiedFd);
				clients.erase(notifiedFd);
				return ;
		}
		std::memset(buffer, '\0', sizeof(buffer));
	}	
}

void	Server::run(Request &req) {
	int					server_fd;
	int					epoll_fd;
	int					n_fds;
	int					opt;
	struct sockaddr_in	server_addr;
	socklen_t			server_len;
	struct epoll_event	ev, events[MAX_EVENTS];

	// Initialization 
	server_addr.sin_family = IPv4;
	server_addr.sin_addr.s_addr = inet_addr(req.config.host.c_str());
	server_addr.sin_port = htons(req.config.listen[0]);
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
	
	 std::cout << "🚀 Server running on port " << req.config.listen[0] << std::endl;
		
	bool	running = true;
	while (running)
	{
		n_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (n_fds < 0)
			throwError("epoll_wait()");
		
		for (int i = 0; i < n_fds ; i++)
		{
			if (events[i].data.fd == server_fd) // case 1: new connection comes, we should accept it
			{
				setUpNewConnection(epoll_fd, server_fd, ev);
			}
			else // case 2: handle client events (read/write/error)
			{
				int fd = events[i].data.fd;
				
				if (events[i].events & EPOLLIN) // Read event => Request received
				{
					if (!recvRequest(epoll_fd, fd, ev))
						continue;
					req.setRequest(clients[fd].request);
				}
				else if (events[i].events & EPOLLOUT) // Write event => Send response
				{
					sendResponse(epoll_fd, fd, req);
				}
				else // Error event => EPOLLERR
				{
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
					close (fd);
					continue;
				}
			}
		}
	}

	close(server_fd);
	close(epoll_fd);
}
