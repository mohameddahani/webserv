/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/26 09:56:59 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// * initialize server name
const std::string Response::serverName = "webserv/1.0\r\n";

// ! Order of response
// HTTP/1.1 200 OK
// Server: webserv/1.0
// Content-Type: text/html
// Content-Length: 128
// Connection: close

// <body>

// * GET METHOD
void Response::GET_METHOD(Request &req) {
  std::string fullPath("pages");

  if (req.path == "/") {
    fullPath += req.path + "index.html";
  } else {
    fullPath += req.path;
  }

  // * check the file permissions and if the file exist
  if (access(fullPath.c_str(), F_OK) == -1) {
    this->setStatusCode(this->NOT_FOUND);
    fullPath = "pages/errors/404.html";
  } else if (access(fullPath.c_str(), R_OK) == -1) {
    this->setStatusCode(this->FORBIDDEN);
    fullPath = "pages/errors/403.html";
  } else {
    this->setStatusCode(this->OK);
  }

  // * status line
  this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));

  // * open file
  std::ifstream file(fullPath.c_str());
  if (!file.is_open()) {
    std::cerr << "Error: file is not open !" << std::endl;
    return;
  }

  // * Content Type
  this->setContentType(fullPath);

  // * Body
  this->setBody(file);

  // * Content Length
  this->setContentLength(this->getBody());

  // * merge all headers
  this->setHeaders();

  // * create response
  this->setResponse();
}

// * POST METHOD
void Response::POST_METHOD() {}

// * DELETE METHOD
void Response::DELETE_METHOD() {}

// * Status code description
std::string Response::statusCodeDescription(STATUS_CODE statusCode) {
  if (statusCode == this->OK) {
    return "200 OK";
  } else if (statusCode == this->CREATED) {
    return "201 Created";
  } else if (statusCode == this->NO_CONTENT) {
    return "204 No Content";
  } else if (statusCode == this->MOVED_PERMANENTLY) {
    return "301 Moved Permanently";
  } else if (statusCode == this->FOUND) {
    return "302 Found";
  } else if (statusCode == this->BAD_REQUEST) {
    return "400 Bad Request";
  } else if (statusCode == this->FORBIDDEN) {
    return "403 Forbidden";
  } else if (statusCode == this->NOT_FOUND) {
    return "404 Not Found";
  } else if (statusCode == this->METHOD_NOT_ALLOWED) {
    return "405 Method Not Allowed";
  } else if (statusCode == this->PAYLOAD_TOO_LARGE) {
    return "413 Payload Too Large";
  } else if (statusCode == this->INTERNAL_SERVER_ERROR) {
    return "500 Internal Server Error";
  } else if (statusCode == this->BAD_GATEWAY) {
    return "502 Bad Gateway";
  } else if (statusCode == this->GATEWAY_TIMEOUT) {
    return "504 Gateway Timeout";
  }
  return "Unknown Status";
}

// * Get content type
void Response::setContentType(const std::string &path) {
  size_t pos = path.rfind(".");

  if (pos == std::string::npos) {
    this->contentType =
        "Content-Type: application/octet-stream\r\n"; // * we dont know what
    // * is the type of
    // * content
    return;
  }

  std::string extention = path.substr(pos);

  std::map<std::string, std::string>::iterator it =
      this->mimeTypes.find(extention);

  if (it == this->mimeTypes.end()) {
    this->contentType =
        "Content-Type: application/octet-stream\r\n"; // * we dont know what
    // * is the type of
    // * content
    return;
  }

  this->contentType = "Content-Type: " + it->second + "\r\n";
}

// * Response
void Response::response(const int clientFd, Request &req) {
  (void)clientFd;

  if (req.method == GET) {
    this->GET_METHOD(req);
  } else if (req.method == POST) {
    /* code */
  } else if (req.method == DELETE) {
    /* code */
  }

  send(clientFd, this->getResponse().c_str(), this->getResponse().length(), 0);
}
