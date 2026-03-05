/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 12:58:19 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/24 22:13:04 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/Server.hpp"

/**
 * @brief Default constructor for the Server class.
 *
 */
Server::Server()
	: _listener(),
	  _epoll(),
	  _connectionManager(_epoll)
{
}

/**
 * @brief Destructor for the Server class.
 *
 * Closes all server listening sockets managed by the Listener.
 * This ensures proper release of file descriptors and prevents
 * resource leaks when the server shuts down.
 */
Server::~Server()
{
	for (size_t i = 0; i < _listener._serverSockets.size(); i++)
	{
		close(_listener._serverSockets[i]);
	}
}

/**
 * @brief Checks for client and CGI timeouts.
 *
 * Iterates over all active clients and:
 * - Terminates CGI processes that exceed CGI_TIMEOUT.
 * - Closes client connections that exceed CLIENT_TIMEOUT.
 *
 * Timed-out connections are properly cleaned up through
 * the ConnectionManager.
 */
void	Server::checkClientTimeOut()
{
	time_t	now = time(NULL);

	for(std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); )
	{
		Client&	client = it->second;

		if (client.isCgiRunning())
		{
			if (now - client.getCgiStartTime() > CGI_TIMEOUT)
			{
				logMessage(LOG_TIMEOUT,
							"CGI timed out for client fd=" +
							toString(client.getClientFd()) +
							"(pid=" + toString(client.getCgiPid()) + ")");

				client.setCgiTimedOut(true);
				handleCgiError(client.getClientFd(), client.getCgiPipeEnd());
			}
		}

		if (now - client.getLastActivity() > CLIENT_TIMEOUT)
		{
			int fd = it->first;
			client.setTimedOut();
			++it;
			_connectionManager.closeConnection(fd, _clients, _cgiPipeToClient, "Client timeout");
		}
		else
			++it;
	}
}

/**
 * @brief Receives incoming data (requests) from a client socket.
 *
 * Delegates the read operation to the ConnectionManager.
 *
 * @param clientFd The client socket file descriptor.
 * @return true if data was successfully received, false on failure or data still in socket.
 */
bool	Server::receiveRequest(int clientFd)
{
	if (!_connectionManager.receiveData(clientFd, _clients, _cgiPipeToClient))
		return (false);
	return (true);
}

/**
 * @brief Sends a response to a client socket.
 *
 * Delegates the write operation to the ConnectionManager.
 *
 * @param clientFd The client socket file descriptor.
 * @param req The associated HTTP request object.
 * @return true if the response was successfully sent, false on failure.
 */
bool	Server::sendResponse(int clientFd, Request& req)
{
	if (!_connectionManager.sendData(clientFd, _clients, _cgiPipeToClient, req))
		return (false);
	return (true);
}

/**
 * @brief Handles an error.
 *
 * Cleanup resources when epoll notifies EPOLLERR.
 * 
 *
 * @param clientFd The client socket file descriptor.
 */
void	Server::handleError(int clientFd)
{
	std::cout << "❌ Error or closing connection for client " << clientFd << std::endl;

	_epoll.delFd(clientFd, "Error OR Client closed connection");
	
	Client& client = _clients[clientFd];
	if (client.getBodyFd() >= 0)
		close(client.getBodyFd());
	close(clientFd);
	_clients.erase(clientFd);
}

/**
 * @brief Processes a new server socket event.
 *
 * Accepts and initializes a new client connection associated
 * with the given server socket file descriptor.
 *
 * @param fd The server listening socket file descriptor.
 */
void	Server::processServerEvent(int fd)
{
	_connectionManager.setUpNewConnection(fd, _clients);
}

/**
 * @brief Checks whether a file descriptor belongs to a CGI pipe.
 *
 * @param fd The file descriptor to check.
 * @return true if the descriptor is associated with a CGI pipe, false otherwise.
 */
bool	Server::isCgiPipeFd(int fd)
{
	return (_cgiPipeToClient.find(fd) != _cgiPipeToClient.end());
}

/**
 * @brief Handles CGI execution errors or timeouts.
 *
 * If the CGI process timed out, it is terminated and a 504 Gateway Timeout
 * response is generated. Otherwise, a 500 Internal Server Error response
 * is prepared.
 *
 * The client socket is switched to EPOLLOUT to send the generated
 * error response.
 *
 * @param clientFd The client socket file descriptor.
 * @param pipeFd The CGI pipe file descriptor associated with the client.
 */
void	Server::handleCgiError(int clientFd, int pipeFd)
{
	Client& client = _clients[clientFd];
	Request& req = _clientRequests[clientFd];
	

	if (client.isCgiTimedOut())
	{
		if (client.getCgiPid() > 0)
		{
			kill(client.getCgiPid(), SIGKILL);
			waitpid(client.getCgiPid(), NULL, 0);
		}
		_epoll.delFd(pipeFd, "CGI timed out");
		close(pipeFd);
		
		std::string	body = loadErrorPage(504);
		
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "text/html";
		headers["Content-Length"] = toString(body.size());
		
		req.setCgiResponse(buildCgiResponse(504, "Gateway Timeout", headers, body));
		
		_epoll.modFd(clientFd, EPOLLOUT);
		_cgiPipeToClient.erase(pipeFd);
		client.setCgiRunning(false);
		client.setStatusCode(Webserv::GATEWAY_TIMEOUT);
	}
	else
	{
		std::string body = loadErrorPage(500);

		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "text/html";
		headers["Content-Length"] = toString(body.size());

		req.setCgiResponse(buildCgiResponse(500, "Internal Server Error", headers, body));
		client.setStatusCode(Webserv::INTERNAL_SERVER_ERROR);
	}
}

/**
 * @brief Handles readable events from a CGI pipe.
 *
 * Reads CGI output when available and appends it to the client buffer.
 * When the CGI process finishes or fails, prepares either a success
 * response (200 OK) or delegates to error handling.
 *
 * The client socket is switched to EPOLLOUT to send the final response.
 *
 * @param pipeFd The CGI pipe file descriptor.
 * @param event The epoll event associated with the pipe.
 */
void Server::handleCgiOutput(int pipeFd, struct epoll_event& event)
{
	if (_cgiPipeToClient.find(pipeFd) == _cgiPipeToClient.end())
		return;

	int clientFd = _cgiPipeToClient[pipeFd];
	Client& client = _clients[clientFd];

	if (!(event.events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
		return;

	char buffer[MAX_BUFFER_SIZE];
	ssize_t bytesRead = _cgiHandler.readChunk(pipeFd, buffer, sizeof(buffer) - 1);

	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		client.appendCgiOutput(std::string(buffer, bytesRead));
		return;
	}

	// CGI finished or failed
	_epoll.delFd(pipeFd, "CGI finished or failed");
	close(pipeFd);

	int exitStatus = 0;
	_cgiHandler.checkCgiStatus(client.getCgiPid(), exitStatus);

	Request& req = _clientRequests[clientFd];

	if (bytesRead < 0 || exitStatus != 0)
		handleCgiError(clientFd, pipeFd);
	else
	{
		CgiResult cgi = parseCgiOutput(client.getCgiOutput());

		req.setCgiResponse(buildCgiResponse(200, "OK", cgi.headers, cgi.body));
		client.setStatusCode(Webserv::OK);
	}

	client.setCgiRunning(false);
	_epoll.modFd(clientFd, EPOLLOUT);
	_cgiPipeToClient.erase(pipeFd);
}

/**
 * @brief Starts CGI execution for a client request.
 *
 * Launches the CGI script associated with the request, registers
 * its pipe in epoll for reading, and links it to the client.
 * If the CGI fails to start, error handling is triggered.
 *
 * @param clientFd The client socket file descriptor.
 * @param req The HTTP request that requires CGI execution.
 */
void	Server::startCgiForClient(int clientFd, const Request& req)
{
	Client&	client = _clients[clientFd];

	pid_t	pid;
	client.setCgiStartTime(time(NULL));
	int pipeFd = _cgiHandler.startCgiScript(req, pid);
	
	if (pipeFd < 0)
	{
		handleCgiError(clientFd, pipeFd);
		_epoll.modFd(clientFd, EPOLLOUT);
		return ;
	} 
	
	client.setCgiPipeEnd(pipeFd);
	client.setCgiPid(pid);
	client.setCgiRunning(true);

	_epoll.addFd(pipeFd, EPOLLIN);

	_cgiPipeToClient[pipeFd] = clientFd;

	logMessage(LOG_CGI,
				"    client fd=" + toString(clientFd) +
				" started (pid=" + toString(pid) + ", pipeFd=" +
				toString(pipeFd) + ")");
}

/**
 * @brief Processes events for a client socket.
 *
 * Handles different types of events:
 * - CGI pipe events are delegated to handleCgiOutput.
 * - Readable events (EPOLLIN) trigger request reception and
 *   possibly start CGI execution.
 * - Writable events (EPOLLOUT) send the prepared response.
 * - Other events are treated as errors and the connection is closed.
 *
 * @param fd The client socket file descriptor.
 * @param event The epoll event associated with the client.
 * @param req The HTTP request object associated with the client.
 */
void	Server::processClientEvent(int fd, struct epoll_event &event, Request &req)
{
	if (isCgiPipeFd(fd))
	{
		handleCgiOutput(fd, event);
		return ;
	}
	if (event.events & EPOLLIN)
	{
		if (receiveRequest(fd))
		{
			req.setRequest(_clients[fd].getRequest());
			_clientRequests[fd] = req;
			logMessage(LOG_REQ,
						"    fd=" + toString(fd) + " " +
						req.getMethodByName(req.method) + " " +
						req.path + " HTTP/1.1");

			if (req.getIsCGI())
				startCgiForClient(fd, req);
			else
				_epoll.modFd(fd, EPOLLOUT);
		}
	}
	else if (event.events & EPOLLOUT)
	{
		Request& request = _clientRequests[fd];
		if (!sendResponse(fd, request))
			return;
	}
	else
	{
		handleError(fd);
		return;
	}
}

/**
 * @brief Runs the server and handles incoming connections and events.
 *
 * Sets up listening sockets based on configuration, registers them
 * with epoll, and enters the main event loop.
 *
 * In the loop:
 * - Accepts new connections on listening sockets.
 * - Processes client events (read/write/CGI).
 * - Checks for client and CGI timeouts.
 *
 * @param req The HTTP request object used for configuration and event handling.
 */
void	Server::run(Request &req)
{
	int n_fds;

	for (size_t i = 0; i < req.config.listen.size(); i++)
	{
		_listener.setHost(req.config.host);
		_listener.setPort(req.config.listen[i]);

		// init server address
		_listener.initServerAddress();

		// socket creation
		_listener.createServerSocket();

		// socket identification
		_listener.bindServerSocket();

		// socket listening
		_listener.startListening();
		
		logMessage(LOG_INFO,
					"   🚀 Server running on " +
					_listener.getHost() + ":" +
					toString(_listener.getPort()));
	}

	// setup server socket to accept new connections
	for(size_t i = 0; i < _listener._serverSockets.size(); i++)
	{
		_epoll.addFd(_listener._serverSockets[i], EPOLLIN);
	}

	// running server and waiting for connections
	struct epoll_event events[MAX_EVENTS];

	bool running = true;
	while (running)
	{
		n_fds = _epoll.wait(events, MAX_EVENTS);

		for (int i = 0; i < n_fds; i++)
		{
			int fd = events[i].data.fd;

			if (_listener.isListeningSocket(fd))
				processServerEvent(fd);
			else
				processClientEvent(fd, events[i], req);
		}

		checkClientTimeOut();
	}
}
