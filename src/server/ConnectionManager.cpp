/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 05:51:49 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/24 22:40:27 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/ConnectionManager.hpp"

/**
 * @brief Default constructor for the ConnectionManager class.
 *
 * @param epoll Reference to the Epoll instance.
 */
ConnectionManager::ConnectionManager(Epoll& epoll)
	: _epollInstance(epoll)
{
}

/**
 * @brief Accepts and initializes a new client connection.
 *
 * Accepts an incoming connection from the listening socket,
 * sets it to non-blocking mode, registers it in epoll for reading,
 * and adds it to the active clients map.
 *
 * @param serverFd The listening server socket file descriptor.
 * @param clients Map of active client connections.
 */
void	ConnectionManager::setUpNewConnection(int serverFd, std::map<int, Client>& clients)
{
	int	clientFd;

	clientFd = accept(serverFd, NULL, NULL);
	if (clientFd < 0)
		return ;

	setNonBlocking(clientFd);

	logMessage(LOG_CONN,
		"   Accepted client fd=" + toString(clientFd) +
		" (server fd=" + toString(serverFd) + ")");

	_epollInstance.addFd(clientFd, EPOLLIN);
	clients.insert(std::make_pair(clientFd, Client(clientFd)));
}

/**
 * @brief Checks whether the HTTP request contains a Content-Length header.
 *
 * Used to determine if the request may include a body and
 * requires further processing.
 *
 * @param request The raw HTTP request string.
 * @return true if "Content-Length:" is found, false otherwise.
 */
bool ConnectionManager::isCompleteRequest(const std::string &request)
{
	if (request.find("Content-Length:") != std::string::npos)
		return (true);
	return (false);
}

/**
 * @brief Extracts the content length value.
 *
 * if the received request is post extracts the content
 * length value, to make sure that the size of body received
 * is the same as content length.
 *
 * @param request The raw HTTP request string.
 * @return content length if found, 0 otherwise.
 */
size_t ConnectionManager::getContentLength(const std::string &request)
{
	size_t pos = request.find("Content-Length:");
	if (pos == std::string::npos)
		return (0);
	size_t start = pos + 15;
	size_t end = request.find("\r\n", start);
	if (end == std::string::npos)
		return (0);

	std::string lengthStr = request.substr(start, end - start);
	return (std::atoll(lengthStr.c_str()));
}

/**
 * @brief Receives and processes incoming data from a client socket.
 *
 * Reads data using recv(), appends it to the client's request buffer,
 * and updates activity timestamps. If the connection is closed or
 * an error occurs, the client is cleaned up.
 *
 * Returns true only when a complete HTTP request (headers and body,
 * if applicable) has been fully received.
 *
 * @param clientFd The client socket file descriptor.
 * @param clients Map of active client connections.
 * @param cgiPipeToClient Map linking CGI pipe descriptors to clients.
 * @return true if the full request has been received, false otherwise.
 */
bool ConnectionManager::receiveData(int clientFd, std::map<int, Client>& clients, std::map<int, int>& cgiPipeToClient)
{
	char buffer[MAX_BUFFER_SIZE];
	ssize_t bytesRead;

	bytesRead = recv(clientFd, buffer, MAX_BUFFER_SIZE - 1, 0);
	if (bytesRead <= 0)
	{
		closeConnection(clientFd, clients, cgiPipeToClient, "Client closed connection");
		return (false);
	}

	Client&	client = clients[clientFd];
	buffer[bytesRead] = '\0';
	client.appendRequest(buffer, bytesRead);
	client.updateLastActivity();
	if (!client.hasCompleteHeaders())
		return (false);
	if (isCompleteRequest(client.getRequest()))
	{
		size_t contentLength = getContentLength(client.getRequest());
		client.setContentLength(contentLength);
		client.setIsPostRequest(contentLength > 0);
	}

	if (!client.hasCompleteBody())
		return (false);

	return (true);
}

/**
 * @brief Sends an HTTP or CGI response to a client.
 *
 * If the request is handled by CGI, sends the prepared CGI response
 * in chunks until completion. Otherwise, sends HTTP headers first,
 * then streams the response body from the associated file descriptor.
 *
 * Handles partial sends and closes the connection once the full
 * response has been transmitted.
 *
 * @param clientFd The client socket file descriptor.
 * @param clients Map of active client connections.
 * @param cgiPipeToClient Map linking CGI pipe descriptors to clients.
 * @param req The HTTP request associated with the client.
 * @return true if the response is fully sent or connection closed,
 *         false if more data remains to be sent.
 */
bool ConnectionManager::sendData(int clientFd, std::map<int, Client>& clients, std::map<int, int>& cgiPipeToClient, Request& req)
{
	if (req.getIsCGI())
	{
		Client&	client = clients[clientFd];
		const std::string&	cgiResp = req.getCgiResponse();
		size_t	remainingBytes = cgiResp.size() - client.getCgiBytesSent();

		ssize_t bytesSent = send(clientFd, cgiResp.c_str() + client.getCgiBytesSent(), remainingBytes, 0);
		if (bytesSent <= 0)
		{
			closeConnection(clientFd, clients, cgiPipeToClient, "failed to send cgi response");
			return (true);
		}
		client.setCgiBytesSent(bytesSent);
		if (client.getCgiBytesSent() == cgiResp.size())
		{
			logMessage(LOG_RESP,
				"   fd=" + toString(clientFd) + " " +
				toString(client.getStatusCode()) + " " +
				statusCodeToString(client.getStatusCode()));
			closeConnection(clientFd, clients, cgiPipeToClient, "cgi response sent");
			return (true);	
		}

		_epollInstance.modFd(clientFd, EPOLLOUT);
		client.updateLastActivity();
		return (false);
	}
	else
	{
		Client&	client = clients[clientFd];
		Response res;
	
		if (!client.isHeaderSent())
		{
				res.response(req);
				std::string responseHeaders = res.getHeaders();

				size_t headersLength = responseHeaders.length();
				ssize_t bytesSent;
		
				bytesSent = send(clientFd, responseHeaders.c_str(), headersLength, 0);
				if (bytesSent <= 0)
				{
					close(clientFd);
					clients.erase(clientFd);
					return (false);
				}
				client.setHeaderSent(true);
				client.setBodyFd(res.getBodyFd());
				client.setStatusCode(res.getStatusCode());
				client.updateLastActivity();
		}
	
		char buffer[MAX_BUFFER_SIZE];
		ssize_t bytesRead;
		ssize_t bytesSent;
	
		bytesRead = read(client.getBodyFd(), buffer, sizeof(buffer));
		if (bytesRead <= 0)
		{
			close(client.getBodyFd());
			logMessage(LOG_RESP,
				"   fd=" + toString(clientFd) + " " +
				toString(client.getStatusCode()) + " " +
				statusCodeToString(client.getStatusCode()));

			closeConnection(clientFd, clients, cgiPipeToClient, "response sent");
			return (true);
		}
	
		bytesSent = send(clientFd, buffer, bytesRead, 0);
		if (bytesSent <= 0)
		{
			close(clientFd);
			clients.erase(clientFd);
			return (false);
		}
	
		client.updateLastActivity();
		return (false);
	}
}

/**
 * @brief Closes and cleans up a client connection.
 *
 * Removes the client socket from epoll, closes the socket,
 * and releases all associated resources. If a CGI process is
 * running, it is terminated and its pipe is cleaned up.
 *
 * The client entry is removed from the active clients map.
 *
 * @param clientFd The client socket file descriptor.
 * @param clients Map of active client connections.
 * @param cgiPipeToClient Map linking CGI pipe descriptors to clients.
 * @param reason Description of why the connection is being closed.
 */
void	ConnectionManager::closeConnection(int clientFd,
											std::map<int, Client>& clients,
											std::map<int, int>& cgiPipeToClient,
											const std::string&	reason)
{
	std::map<int, Client>::iterator it = clients.find(clientFd);
	if (it == clients.end())
		return;

	_epollInstance.delFd(clientFd, reason);
	close(clientFd);

	if (it->second.getIsTimedOut())
		logMessage(LOG_TIMEOUT, "client fd=" + toString(clientFd) + " timed out");

	if (it->second.isCgiRunning())
	{
		pid_t	pid = it->second.getCgiPid();
		int		pipeFd = it->second.getCgiPipeEnd();
		if (pid > 0)
		{
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
		}
		_epollInstance.delFd(pipeFd, reason);
		close(pipeFd);
		cgiPipeToClient.erase(pipeFd);
	}

	clients.erase(it);

	logMessage(LOG_DEBUG, "  client fd=" + toString(clientFd) + " removed from clients map");
}