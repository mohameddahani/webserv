/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiUtils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 11:04:26 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 10:24:16 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CgiUtils.hpp"

/**
 * @brief Trims leading and trailing whitespace characters from a string.
 *
 * Removes spaces, tabs, carriage returns, and newlines from both ends.
 *
 * @param str The input string to trim.
 * @return A new string with no leading or trailing whitespace.
 */
std::string	ft_trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

/**
 * @brief Parses raw CGI headers into a key-value map.
 *
 * - Splits the input string by lines.
 * - Ignores empty lines or lines without a colon.
 * - Trims whitespace from names and values.
 * - Converts header names to lowercase.
 *
 * @param rawHeaders The raw CGI header string.
 * @return A map of header names to values.
 */
std::map<std::string, std::string>	parseCgiHeaders(const std::string& rawHeaders) {
	std::map<std::string, std::string> headers;
	std::istringstream stream(rawHeaders);
	std::string line;
		
	while (std::getline(stream, line)) {

		if (!line.empty() && line[line.length() - 1] == '\r') {
			line = line.substr(0, line.length() - 1);
		}

		if (line.empty())
			continue;

		size_t colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			continue;

		std::string header_name = ft_trim(line.substr(0, colon_pos));
		std::string header_value = ft_trim(line.substr(colon_pos + 1));

		for (size_t i = 0; i < header_name.length(); i++) {
			header_name[i] = std::tolower(header_name[i]);
		}

		if (!header_name.empty()) {
			headers[header_name] = header_value;
		}
	}

	return headers;
}

/**
 * @brief Returns the file path of a custom or default error page.
 *
 * Checks if a custom error page exists for the given status code
 * under "www/errors/". If not, falls back to the default page.
 *
 * @param statusCode The HTTP status code (e.g., 500, 504).
 * @return File path to the corresponding error page.
 */
std::string	getErrorPagePath(int statusCode)
{
	std::string	customizedErrorPage;

	if (statusCode == 500)
	{
		customizedErrorPage = "www/errors/500.html";
		if (access(customizedErrorPage.c_str(), R_OK) == 0)
			return (customizedErrorPage);
		return ("defaults/errors/500.html");
	}
	if (statusCode == 504)
	{
		customizedErrorPage = "www/errors/504.html";
		if (access(customizedErrorPage.c_str(), R_OK) == 0)
			return (customizedErrorPage);
		return ("defaults/errors/504.html");
	}

	return ("");
}

/**
 * @brief Loads the content of the error page for a given status code.
 *
 * Determines the correct error page path (custom or default)
 * and reads its contents into a string.
 *
 * @param statusCode The HTTP status code (e.g., 500, 504).
 * @return The content of the error page as a string.
 */
std::string	loadErrorPage(int statusCode)
{
	std::string	path;
	
	if (statusCode == 500)
		path = getErrorPagePath(statusCode);
	else if (statusCode == 504)
		path = getErrorPagePath(statusCode);
	else
		path = "defaults/errors/503.html";

	return (Helper::readFile(path));
}

/**
 * @brief Parses raw CGI output into headers and body.
 *
 * Splits the output by the header-body separator (`\r\n\r\n` or `\n\n`),
 * parses the headers, and ensures `Content-Type` and `Content-Length`
 * are set if missing.
 *
 * @param raw The raw output string from a CGI script.
 * @return A CgiResult struct containing parsed headers and body.
 */
CgiResult	parseCgiOutput(const std::string& raw)
{
	CgiResult	result;

	size_t	headerEnd = raw.find("\r\n\r\n");
	size_t	sepSize = 4;

	if (headerEnd == std::string::npos)
	{
		headerEnd = raw.find("\n\n");
		sepSize = 2;
	}

	if (headerEnd != std::string::npos)
	{
		std::string	headersStr = raw.substr(0, headerEnd);
		result.headers = parseCgiHeaders(headersStr);
		result.body = raw.substr(headerEnd + sepSize);
	}
	else
		result.body = raw;
	if (result.headers.find("content-type") == result.headers.end())
	{
		 result.headers["Content-Type"] = "text/plain";
	}
	
	if (result.headers.find("content-length") == result.headers.end())
	{
		result.headers["Content-Length"] = Helper::toString(result.body.size());
	}

	return (result);
}

/**
 * @brief Builds a complete HTTP response from CGI results.
 *
 * Constructs the HTTP status line, headers, and body into a single string
 * suitable for sending to the client.
 *
 * @param statusCode The HTTP status code (e.g., 200, 500, 504).
 * @param reason The reason phrase corresponding to the status code.
 * @param headers Map of HTTP headers to include.
 * @param body The body of the response.
 * @return A formatted HTTP response string.
 */
std::string	buildCgiResponse(int statusCode,
										const std::string& reason,
										const std::map<std::string, std::string>& headers,
										const std::string& body)
{
	std::ostringstream oss;

	oss << "HTTP/1.1 " << statusCode << " " << reason << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "Connection: close\r\n";
	oss << "\r\n";
	oss << body;

	return (oss.str());
}