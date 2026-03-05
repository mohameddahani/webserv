/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helper.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 07:06:49 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 10:34:00 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/Helper.hpp"

/**
 * @brief Sets a file descriptor to non-blocking mode.
 *
 * @param fd The file descriptor to modify.
 * @throws std::runtime_error if fcntl fails.
 */
void	Helper::setNonBlocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throwError("fcntl(F_GETFL)");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throwError("fcntl(F_SETFL)");
}

/**
 * @brief Throws a runtime error with a system call message.
 *
 * Logs the error and throws a std::runtime_error with details.
 *
 * @param sysCallFunc Name of the system call or function that failed.
 * @throws std::runtime_error.
 */
void	Helper::throwError(const std::string& sysCallFunc)
{
	logMessage(LOG_ERROR, "  " + sysCallFunc + " has been failed");
	throw std::runtime_error(sysCallFunc + " failed: " + strerror(errno));
}

/**
 * @brief Reads the entire content of a file into a string.
 *
 * If the file cannot be opened, returns a default 500 Internal Server Error HTML page.
 *
 * @param path Path to the file.
 * @return File content as a string.
 */
std::string	Helper::readFile(const std::string& path)
{
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

	if (!file.is_open())
		return ("<html>"
				"<head><title>500 Internal Server Error</title></head>"
				"<body>"
				"<center><h1>500 Internal Server Error</h1></center>"
				"<hr><center>webserv/1.0</center>"
				"</body>"
				"</html>");

	std::ostringstream ss;
	ss << file.rdbuf();

	return ss.str();
}

/**
 * @brief Converts a numeric value to a string.
 *
 * @param val The value to convert.
 * @return String representation of the input value.
 */
std::string	Helper::toString(size_t val)
{
	std::stringstream	ss;

	ss << val;
	return (ss.str());
}