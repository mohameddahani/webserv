/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 13:13:08 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 09:47:17 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "server/Epoll.hpp"
#define CLIENT_TIMEOUT 30 // 30s


// ************************************************************************** //
//                                 Client Class                               //
// ************************************************************************** //

class Client : public Webserv {

	public:
		Client();
		Client(int clientFd);

		void					appendRequest(const char *buffer, size_t length);
		void					updateLastActivity();
		size_t					findHeaderEnd() const;
		bool					hasCompleteHeaders() const;
		bool					hasCompleteBody() const;

		void					setIsPostRequest(bool val);
		void					setRequestComplete(bool val);
		void					setHeaderSent(bool val);
		void					setContentLength(size_t length);
		void					setBodyFd(int fd);
		void					setTimedOut();
		void					setStatusCode(STATUS_CODE statusCode);

		const std::string&		getRequest() const;
		bool					isHeaderSent() const;
		int						getBodyFd() const;
		time_t					getLastActivity() const;
		int						getClientFd() const;
		bool					getIsTimedOut() const;
		Webserv::STATUS_CODE	getStatusCode() const;

		// cgi methods
		void					setCgiPipeEnd(int fd);
		int						getCgiPipeEnd() const;
		void					setCgiPid(pid_t pid);
		pid_t					getCgiPid() const;
		void					setCgiRunning(bool val);
		bool					isCgiRunning() const;
		void					appendCgiOutput(const std::string& data);
		const std::string&		getCgiOutput() const;
		void					setCgiStartTime(time_t val);
		time_t					getCgiStartTime() const;
		void					setCgiTimedOut(bool val);
		bool					isCgiTimedOut() const;
		size_t					getCgiBytesSent() const;
		void					setCgiBytesSent(size_t val);

	private:
		std::string				_request;
		size_t					_bytesReceived;
		size_t					_contentLength;
		time_t					_lastActivity;
		bool					_isPostRequest;
		bool					_requestComplete;
		bool					_isHeaderSent;
		bool					_isTimedOut;
		int						_clientFd;
		int						_bodyFd;
		Webserv::STATUS_CODE	_statusCode;

		// cgi state
		int						_cgiPipeFd;
		pid_t					_cgiPid;
		bool					_isCgiRunning;
		bool					_isCgiTimedOut;
		std::string				_cgiOutput;
		time_t					_cgiStartTime;
		size_t					_cgiBytesSent;

};