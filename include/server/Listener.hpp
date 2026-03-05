/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 17:53:16 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 08:03:42 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/Helper.hpp"
#define BACK_LOG 1024


// ************************************************************************** //
//                                Listener Class                              //
// ************************************************************************** //

class Listener : Helper {

	public:
		std::vector<int>	_serverSockets;
		Listener();
		~Listener();

		void		initServerAddress();
		void		createServerSocket();
		void		bindServerSocket();
		void		startListening();
		bool		isListeningSocket(int fd);
		std::string	getHost() const;
		int			getSockFd() const;
		int			getPort() const;
		void		setHost(std::string& host);	
		void		setSockFd(int fd);
		void		setPort(int port);

		void		throwError(std::string func);

	private:
		struct sockaddr_in	_serverAddr;
		std::string			_host;
		int					_sockfd;
		int					_port;
};

