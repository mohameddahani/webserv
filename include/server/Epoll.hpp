/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 10:46:25 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 08:46:51 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Listener.hpp"
#define EPOLL_TIMEOUT 1000 // 1s


// ************************************************************************** //
//                                 Epoll Class                                //
// ************************************************************************** //

class Epoll : Helper {

	public:
		Epoll();
		~Epoll();

		void	addFd(int fd, uint32_t event);
		void	modFd(int fd, uint32_t event);
		void	delFd(int fd, const std::string& reason);
		int		wait(struct epoll_event *events, int maxEvents);

		int		getEpollFd() const;

		void	logEpollAdd(int fd);
		void	logEpollMod(int fd, uint32_t event);
		void	logepollDel(int fd, const std::string& reason);

	private:
		int	_epollfd;

};