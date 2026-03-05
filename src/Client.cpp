/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 13:13:27 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 09:45:41 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

/**
 * @brief Default constructor for Client class.
 *
 */
Client::Client()
	: _clientFd(-1)
{	
}

/**
 * @brief Parametrise constructor for Client class.
 *
 * Initializes all request, response, timeout, and CGI-related
 * state variables to their default values.
 *
 * @param clientFd The client socket file descriptor.
 */
Client::Client(int clientFd)
	: _bytesReceived(0),
	  _contentLength(0),
	  _lastActivity(time(NULL)),
	  _isPostRequest(false),
	  _requestComplete(false),
	  _isHeaderSent(false),
	  _isTimedOut(false),
	  _clientFd(clientFd),
	  _bodyFd(-1),
	  _cgiPipeFd(-1),
	  _cgiPid(-1),
	  _isCgiRunning(false),
	  _isCgiTimedOut(false),
	  _cgiStartTime(-1),
	  _cgiBytesSent(0)
{
}


/**
 * @brief setter
 *  
 * Sets whether the request is a POST request.
 * 
 * @param val The new value to be setted
 */
void	Client::setIsPostRequest(bool val)
{
	_isPostRequest = val;
}

/**
 * @brief setter 
 * 
 * Marks the request as complete or incomplete.
 * 
 * @param val The new value to be setted
 */
void	Client::setRequestComplete(bool val)
{
	_requestComplete = val;
}

/**
 * @brief setter 
 * 
 * Sets the expected content length of the request body.
 * 
 * @param length length of body request
 */
void	Client::setContentLength(size_t length)
{
	_contentLength = length;
}

/**
 * @brief setter 
 * 
 * Marks whether response headers have been sent.
 * 
 * @param val The new value to be setted
 */
void	Client::setHeaderSent(bool val)
{
	_isHeaderSent = val;
}

/**
 * @brief setter 
 * 
 * Sets the file descriptor of the response body.
 * 
 * @param fd The file descriptor associated to response body
 */
void	Client::setBodyFd(int fd)
{
	_bodyFd = fd;
}

/**
 * @brief setter 
 * 
 * Updates the client's last activity timestamp.
 */
void	Client::updateLastActivity()
{
	_lastActivity = time(NULL);
}

/**
 * @brief setter 
 * 
 * Marks the client connection as timed out.
 */
void	Client::setTimedOut()
{
	_isTimedOut = true;
}

/**
 * @brief setter 
 * 
 * Sets the HTTP status code for the client response.
 * 
 * @param statusCode The status code returned in response.
 */
void	Client::setStatusCode(STATUS_CODE statusCode)
{
	_statusCode = statusCode;
}

/**
 * @brief setter 
 * 
 * Sets the CGI pipe file descriptor.
 * 
 * @param fd The pipe read end file descriptor.
 */
void	Client::setCgiPipeEnd(int fd)
{
	_cgiPipeFd = fd;
}

/**
 * @brief setter 
 * 
 * Sets the CGI process ID.
 * 
 * @param pid The process ID.
 */
void	Client::setCgiPid(pid_t pid)
{
	_cgiPid = pid;
}

/**
 * @brief setter 
 * 
 * Marks whether a CGI process is currently running.
 * 
 * @param val The new value to be setted.
 */
void	Client::setCgiRunning(bool val)
{
	_isCgiRunning = val;
}

/**
 * @brief setter 
 * 
 * Sets the CGI start time.
 * 
 * @param val The new value to be setted.
 */
void	Client::setCgiStartTime(time_t val)
{
	_cgiStartTime = val;
}

/**
 * @brief setter 
 * 
 * Marks whether the CGI execution timed out.
 * 
 * @param val The new value to be setted.
 */
void	Client::setCgiTimedOut(bool val)
{
	_isCgiTimedOut = val;
}

/**
 * @brief setter 
 * 
 * Updates the number of bytes sent for the CGI response.
 * 
 * @param val The new value to be appended.
 */
void	Client::setCgiBytesSent(size_t val)
{
	_cgiBytesSent += val;
}

/**
 * @brief getter
 *
 * @return The raw request received.
 */
const std::string&	Client::getRequest() const
{
	return (_request);
}

/**
 * @brief getter
 *
 * @return true if headers are sent, otherwise false.
 */
bool	Client::isHeaderSent() const
{
	return (_isHeaderSent);
}

/**
 * @brief getter
 *
 * @return The body file descriptor associated to response body.
 */
int	Client::getBodyFd() const
{
	return (_bodyFd);
}

/**
 * @brief getter
 *
 * @return The client's last activity timestamp.
 */
time_t	Client::getLastActivity() const
{
	return (_lastActivity);
}

/**
 * @brief getter
 *
 * @return The client socket file descriptor.
 */
int		Client::getClientFd() const
{
	return (_clientFd);
}

/**
 * @brief getter
 *
 * @return Whether the client connection timed out.
 */
bool	Client::getIsTimedOut() const
{
	return (_isTimedOut);
}

/**
 * @brief getter
 *
 * @return The client's HTTP status code.
 */
Webserv::STATUS_CODE	Client::getStatusCode() const
{
	return (_statusCode);
}

/**
 * @brief getter
 *
 * @return The CGI pipe read end file descriptor.
 */
int	Client::getCgiPipeEnd() const
{
	return (_cgiPipeFd);
}

/**
 * @brief getter
 *
 * @return The CGI process ID.
 */
pid_t	Client::getCgiPid() const
{
	return (_cgiPid);
}

/**
 * @brief getter
 *
 * @return Whether a CGI process is running.
 */
bool	Client::isCgiRunning() const
{
	return (_isCgiRunning);
}

/**
 * @brief getter
 *
 * @return The collected CGI output.
 */
const std::string& Client::getCgiOutput() const
{
	return (_cgiOutput);
}

/**
 * @brief getter
 *
 * @return The CGI start time.
 */
time_t	Client::getCgiStartTime() const
{
	return (_cgiStartTime);
}

/**
 * @brief getter
 *
 * @return Whether the CGI execution timed out.
 */
bool	Client::isCgiTimedOut() const
{
	return (_isCgiTimedOut);
}

/**
 * @brief getter
 *
 * @return The number of bytes sent for the CGI response.
 */
size_t	Client::getCgiBytesSent() const
{
	return (_cgiBytesSent);
}

/**
 * @brief Appends data to the CGI output buffer.
 *
 * @param data String data read from the CGI process.
 */
void	Client::appendCgiOutput(const std::string& data)
{
	_cgiOutput += data;
}

/**
 * @brief Appends received request data to the client's buffer.
 *
 * Updates the total bytes received.
 *
 * @param buffer Pointer to the received data.
 * @param length Number of bytes to append.
 */
void	Client::appendRequest(const char* buffer, size_t length)
{
	_request.append(buffer, length);
	_bytesReceived += length;
}

/**
 * @brief Finds the end of the HTTP headers in the request.
 *
 * Looks for the "\r\n\r\n" sequence marking the end of headers.
 *
 * @return Position of the header end, or std::string::npos if not found.
 */
size_t	Client::findHeaderEnd() const
{
	return (_request.find("\r\n\r\n"));
}

/**
 * @brief Checks if the HTTP headers have been fully received.
 *
 * @return true if the headers end sequence "\r\n\r\n" is found, false otherwise.
 */
bool	Client::hasCompleteHeaders() const
{
	return (_request.find("\r\n\r\n") != std::string::npos);
}


/**
 * @brief Checks if the HTTP request body has been fully received.
 *
 * - Returns false if headers are incomplete.
 * - Returns true immediately for non-POST requests.
 * - For POST requests, returns true only if the body size matches or exceeds Content-Length.
 *
 * @return true if the complete body is received, false otherwise.
 */
bool	Client::hasCompleteBody() const
{
	size_t	headerEnd = findHeaderEnd();
	if (headerEnd == std::string::npos)
		return (false);

	if (!_isPostRequest)
		return (true);

	size_t bodyStart = headerEnd + 4;
	size_t currentBodySize = _request.length() - bodyStart;
	return (currentBodySize >= _contentLength);
}
