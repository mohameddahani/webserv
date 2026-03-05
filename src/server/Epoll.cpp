/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 10:46:40 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 08:45:26 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server/Epoll.hpp"

/**
 * @brief Default constructor for the Epoll class.
 *
 * Creates the epoll instance used for I/O multiplexing.
 *
 * @throws Runtime error if epoll_create() fails.
 */
Epoll::Epoll()
	: _epollfd(-1)
{
	_epollfd = epoll_create(1024);
	if (_epollfd < 0)
		throwError("epoll_create()");
}

/**
 * @brief Destructor for the Epoll class.
 *
 * Closes the epoll file descriptor.
 */
Epoll::~Epoll()
{
	if (_epollfd >= 0)
		close (_epollfd);
}

/**
 * @brief Adds a file descriptor to the epoll instance.
 *
 * Registers the given file descriptor with the specified
 * epoll events (e.g., EPOLLIN, EPOLLOUT).
 *
 * @param fd The file descriptor to monitor.
 * @param event Bitmask of epoll events.
 *
 * @throws Runtime error if epoll_ctl fails.
 */
void	Epoll::addFd(int fd, uint32_t event)
{
	struct epoll_event ev;

	ev.events = event;
	ev.data.fd = fd;

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev) < 0)
		throwError("epoll_ctl(EPOLL_CTL_ADD)");

	logEpollAdd(fd);
}

/**
 * @brief Modifies the events monitored for a file descriptor.
 *
 * Updates the epoll event mask for an already registered
 * file descriptor.
 *
 * @param fd The file descriptor to modify.
 * @param events New epoll event bitmask.
 *
 * @throws Runtime error if epoll_ctl fails.
 */
void	Epoll::modFd(int fd, uint32_t events)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev) < 0)
		throwError("epoll_ctl(EPOLL_CTL_MOD)");

	logEpollMod(fd, events);
}

/**
 * @brief Removes a file descriptor from the epoll instance.
 *
 * Unregisters the file descriptor and logs the reason
 * for its removal.
 *
 * @param fd The file descriptor to remove.
 * @param reason Description of why the descriptor is removed.
 *
 * @throws Runtime error if epoll_ctl fails.
 */
void	Epoll::delFd(int fd, const std::string& reason)
{
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) < 0)
		throwError("epoll_ctl(EPOLL_CTL_DEL)");

	logepollDel(fd, reason);
}

/**
 * @brief Waits for events on registered file descriptors.
 *
 * Blocks until events occur or the configured timeout expires.
 *
 * @param events Array to store triggered events.
 * @param maxEvents Maximum number of events to process.
 * @return Number of ready file descriptors.
 *
 * @throws Runtime error if epoll_wait fails.
 */
int	Epoll::wait(struct epoll_event *events, int maxEvents)
{
	int nfds = epoll_wait(_epollfd, events, maxEvents, EPOLL_TIMEOUT);
	if (nfds < 0)
		throwError("epoll_wait()");
	return (nfds);
}

/**
 * @brief getter
 *
 * @return The epoll file descriptor.
 */
int	Epoll::getEpollFd() const
{
	return (_epollfd);
}

/**
 * @brief Logs the addition of a file descriptor to epoll.
 *
 * @param fd The file descriptor added.
 */
void	Epoll::logEpollAdd(int fd)
{
	logMessage(LOG_EPOLL, "  fd=" + toString(fd) + " added");
}

/**
 * @brief Logs the modification of a file descriptor in epoll.
 *
 * @param fd The file descriptor modified.
 * @param event The new epoll event mask.
 */
void	Epoll::logEpollMod(int fd, uint32_t event)
{
	if (event == EPOLLIN)
		logMessage(LOG_EPOLL, "  fd=" + toString(fd) + " modified (EPOLLIN)");
	if (event == EPOLLOUT)
		logMessage(LOG_EPOLL, "  fd=" + toString(fd) + " modified (EPOLLOUT)");
}

/**
 * @brief Logs the removal of a file descriptor from epoll.
 *
 * @param fd The file descriptor removed.
 * @param reason Reason for removal.
 */
void	Epoll::logepollDel(int fd, const std::string& reason)
{
	logMessage(LOG_EPOLL, "  fd=" + toString(fd) + " removed (" + reason + ")");
}