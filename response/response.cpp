/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/24 20:05:35 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// ! Order of response
// HTTP/1.1 200 OK
// Date: Wed, 24 Dec 2025 15:30:00 GMT
// Server: webserv/1.0
// Content-Type: text/html
// Content-Length: 128
// Connection: close

// <body>

// * GET METHOD
void Response::GET_METHOD() {}

// * POST METHOD
void Response::POST_METHOD() {}

// * DELETE METHOD
void Response::DELETE_METHOD() {}

void response(int clientFd, std::string req) {
  (void)clientFd;
  (void)req;
  std::stringstream ss(req);
  std::string res = "";
  std::string method = "";
  std::string path = "";

  // * get method
  ss >> method;

  // * get path
  ss >> path;
}
