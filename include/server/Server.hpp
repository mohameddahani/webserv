/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/24 22:17:32 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../http/Response.hpp"
#include "../cgi/CgiHandler.hpp"


// ************************************************************************** //
//                                  Server Class                              //
// ************************************************************************** //

class Server : Helper {

	public:
		Server();
		~Server();

		void	run(Request &req);
	
	private:
		std::map<int, Client>	_clients;
		std::map<int, Request>	_clientRequests;
		std::map<int, int>		_cgiPipeToClient;
		Listener				_listener;
		Epoll					_epoll;
		ConnectionManager		_connectionManager;
		CgiHandler				_cgiHandler;

		// private helper methods
		void	checkClientTimeOut();
		
		bool	receiveRequest(int clientFd);
		bool	sendResponse(int clientFd, Request& req);
		void	handleError(int clientFd);

		void	processClientEvent(int fd, struct epoll_event &ev, Request &req);
		void	processServerEvent(int fd);

		// cgi methods
		void	startCgiForClient(int clientFd, const Request& req);
		void	handleCgiOutput(int pipeFd, struct epoll_event& event);
		void	handleCgiError(int clientFd, int pipeFd);
		bool	isCgiPipeFd(int fd);
};
