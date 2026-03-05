/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 05:51:20 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/24 22:43:01 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Client.hpp"
#include "../http/Response.hpp"


// ************************************************************************** //
//                           ConnectionManager Class                          //
// ************************************************************************** //

class ConnectionManager : Helper, Webserv {

	public:
		ConnectionManager(Epoll& epoll);
		
		void	setUpNewConnection(int serverFd, std::map<int, Client>& clients);
		bool	isCompleteRequest(const std::string &request);
		size_t	getContentLength(const std::string &request);
		bool	receiveData(int clientFd, std::map<int, Client>& clients, std::map<int, int>& cgiPipeToClient);
		bool	sendData(int clientFd, std::map<int, Client>& clients, std::map<int, int>& cgiPipeToClient, Request& req);
		void	closeConnection(int clientFd,
								std::map<int,Client>& clients,
								std::map<int, int>& cgiPipeToClient,
								const std::string& reason);

	private:
		Epoll	_epollInstance;
};