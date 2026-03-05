/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 17:53:32 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 08:01:59 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/Listener.hpp"

/**
 * @brief Default constructor for the Listener class.
 *
 */
Listener::Listener() : _host("127.0.0.1"), _sockfd(-1), _port(-1)
{
	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

/**
 * @brief Destructor for the Listener class.
 *
 * Iterates through all listening sockets and closes their
 * file descriptors to release system resources.
 */
Listener::~Listener()
{
	for (size_t i = 0; i < _serverSockets.size(); i++)
	{
		close(_serverSockets[i]);
	}
}

/**
 * @brief Initializes the server address structure.
 *
 * Configures the sockaddr_in structure with the current
 * host and port, preparing it for socket binding.
 */
void	Listener::initServerAddress()
{
	_serverAddr.sin_family = IPv4;
	_serverAddr.sin_addr.s_addr = inet_addr(_host.c_str());
	_serverAddr.sin_port = htons(_port);
	std::memset(_serverAddr.sin_zero, 0, sizeof(_serverAddr.sin_zero));
}

/**
 * @brief Creates and configures the server socket.
 *
 * Creates a TCP socket, enables SO_REUSEADDR, and sets
 * it to non-blocking mode.
 *
 * @throws Runtime error if socket creation or configuration fails.
 */
void	Listener::createServerSocket()
{
	int	opt = 1;

	_sockfd = socket(IPv4, SOCK_STREAM, 0);
	if (_sockfd < 0)
		throwError("socket()");
		
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throwError("setsockopt()");
	
	setNonBlocking(_sockfd);
}

/**
 * @brief Binds the server socket to the configured address and port.
 *
 * Associates the socket with the sockaddr_in structure prepared
 * in initServerAddress().
 *
 * @throws Runtime error if binding fails.
 */
void	Listener::bindServerSocket()
{
	if (bind(_sockfd, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
		throwError("bind()");
}

/**
 * @brief Puts the server socket into listening mode.
 *
 * Starts listening for incoming connections using the
 * configured backlog value and stores the socket in the
 * list of active server sockets.
 *
 * @throws Runtime error if listen() fails.
 */
void	Listener::startListening()
{
	if (listen(_sockfd, BACK_LOG) < 0)
		throwError("listen()");
	
	_serverSockets.push_back(_sockfd);
}

/**
 * @brief Checks whether a file descriptor is a listening socket.
 *
 * Determines if the given file descriptor belongs to
 * the list of active server sockets.
 *
 * @param fd The file descriptor to check.
 * @return true if it is a listening socket, false otherwise.
 */
bool	Listener::isListeningSocket(int fd)
{
	if (std::find(_serverSockets.begin(), _serverSockets.end(), fd) != _serverSockets.end())
		return (true);
	return (false);
}

/**
 * @brief setter
 *
 * Sets the host address for the listener.
 *
 * @param host The host IP address.
 */
void	Listener::setHost(std::string& host)
{
	_host = host;
}

/**
 * @brief setter
 *
 * Sets the socket file descriptor.
 *
 * @param fd The socket file descriptor.
 */
void	Listener::setSockFd(int fd)
{
	_sockfd = fd;
}

/**
 * @brief setter
 *
 * Sets the port number for the listener.
 *
 * @param port The port number.
 */
void	Listener::setPort(int port)
{
	_port = port;
}

/**
 * @brief getter
 *
 * @return The host IP address.
 */
std::string	Listener::getHost() const
{
	return (_host);
}

/**
 * @brief getter
 *
 * @return The socket file descriptor.
 */
int	Listener::getSockFd() const
{
	return (_sockfd);
}

/**
 * @brief getter
 *
 * @return The port number.
 */
int	Listener::getPort() const
{
	return (_port);
}

/**
 * @brief Throws a runtime error with the current errno message.
 *
 * @param func The name of the function where the error occurred.
 *
 * @throws std::runtime_error Always thrown with detailed error message.
 */
void	Listener::throwError(std::string func)
{
  throw std::runtime_error(func + "failed: " + strerror(errno));
}