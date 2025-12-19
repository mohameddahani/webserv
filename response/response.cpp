/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/19 20:43:37 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

void response(int clientFd) {
  std::string statusCode = "HTTP/1.1 200 OK\r\n";
  std::string contentLength = "Content-Length: ";
  std::string contentType = "Content-Type: text/html\r\n";
  std::string res = "";

  std::ifstream file("../pages/index.html");

  std::string line;
  std::string body;

  while (std::getline(file, line)) {
    body += line + '\n';
  }

  std::stringstream ss;
  ss << body.length();
  contentLength += ss.str() + "\r\n";

  res += statusCode + contentLength + contentType + "\r\n" + body;

  send(clientFd, res.c_str(), res.length(), 0);
}
