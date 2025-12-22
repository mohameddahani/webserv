/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/21 09:23:05 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// * GET METHOD
void Response::GET_METHOD() {}

// * POST METHOD
void Response::POST_METHOD() {}

// * DELETE METHOD
void Response::DELETE_METHOD() {}

void response(int clientFd, std::string req) {
  std::string res = "";
  std::string method = "";
  std::string path = "";

  // * get method
  size_t i = 0;
  for (; i < req.length(); i++) {
    if (std::isspace(req[i])) {
      break;
    }
    method += req[i];
  }

  // * get path
  for (i++; i < req.length(); i++) {
    if (std::isspace(req[i])) {
      break;
    }
    path += req[i];
  }

  if (method == "GET") {
    std::string statusCode = "HTTP/1.1 200 OK\r\n";
    std::string contentLength = "Content-Length: ";
    std::string contentType = "Content-Type: text/html\r\n";

    std::string line;
    std::string body;
    std::string filePath = "../pages";

    if (path == "/") {
      filePath += path + "index.html";
      std::ifstream file(filePath.c_str());
      if (!file.is_open()) {
        // throw std::runtime_error("404 NOT FOUND !");
      }

      while (std::getline(file, line)) {
        body += line + '\n';
      }
    } else {
      filePath += path + ".html";
      std::ifstream file(filePath.c_str());
      if (!file.is_open()) {
        // throw std::runtime_error("404 NOT FOUND !");
      }

      while (std::getline(file, line)) {
        body += line + '\n';
      }
    }

    std::stringstream ss;
    ss << body.length();
    contentLength += ss.str() + "\r\n";

    res += statusCode + contentLength + contentType + "\r\n" + body;
  } else if (method == "POST") {
  } else if (method == "DELETE") {
  } else {
    throw std::runtime_error("Error: Unkown Method !");
  }

  send(clientFd, res.c_str(), res.length(), 0);
}
