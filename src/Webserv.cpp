/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 13:13:15 by mdahani           #+#    #+#             */
/*   Updated: 2026/02/25 09:51:27 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Webserv.hpp"

/**
 * @brief Converts an HTTP status code enum to its standard string representation.
 *
 * Maps Webserv::STATUS_CODE values to their corresponding
 * human-readable status messages (e.g., 200 → "OK").
 *
 * @param code The HTTP status code enum.
 * @return A string describing the status code.
 */
std::string Webserv::statusCodeToString(STATUS_CODE code)
{
    switch (code)
    {
        case OK:                     return "OK";
        case CREATED:                return "Created";
        case NO_CONTENT:             return "No Content";

        case MOVED_PERMANENTLY:      return "Moved Permanently";
        case FOUND:                  return "Found";

        case BAD_REQUEST:            return "Bad Request";
        case FORBIDDEN:              return "Forbidden";
        case NOT_FOUND:              return "Not Found";
        case METHOD_NOT_ALLOWED:     return "Method Not Allowed";
        case PAYLOAD_TOO_LARGE:      return "Payload Too Large";

        case INTERNAL_SERVER_ERROR:  return "Internal Server Error";
        case BAD_GATEWAY:            return "Bad Gateway";
        case SOMTHING_WENT_WRONG:    return "Service Unavailable";
        case GATEWAY_TIMEOUT:        return "Gateway Timeout";

        default:
            return "Unknown Status";
    }
}