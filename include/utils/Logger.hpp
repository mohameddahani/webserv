/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/28 09:39:18 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 10:37:45 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <string>
#define CLR_RESET       "\033[0m"
#define CLR_RED         "\033[31m"
#define CLR_GREEN       "\033[32m"
#define CLR_YELLOW      "\033[33m"
#define CLR_BLUE        "\033[34m"
#define CLR_MAGENTA     "\033[35m"
#define CLR_CYAN        "\033[36m"
#define CLR_GRAY        "\033[90m"
#define CLR_BOLD_YELLOW "\033[1;33m"


enum LogTag {
    LOG_INFO,
    LOG_EPOLL,
    LOG_CONN,
    LOG_REQ,
    LOG_RESP,
    LOG_CLOSE,
    LOG_CGI,
    LOG_TIMEOUT,
	LOG_DEBUG,
    LOG_ERROR
};

extern	bool debug_mode;

void		logMessage(LogTag tag, const std::string& msg);
const char*	tagToStr(LogTag tag);
const char*	tagToColor(LogTag tag);

