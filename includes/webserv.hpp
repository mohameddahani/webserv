/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/26 09:54:55 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// * includes
#include <climits>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

// * MACROS
#define PORT 8080
#define IP INADDR_ANY
#define MAX_LISTEN 4096
#define MAX_BUFFER_SIZE 4096
#define MAX_EVENTS 1024

// * classes
// ! WEBSERV
class Webserv {
  // ! private
private:
  // ! public
public:
  std::map<std::string, std::string> request;
  std::map<std::string, std::string> mimeTypes;
  // std::map<std::string, std::string> response;

  // * Default Constructor
  Webserv() { this->initMimeTypes(); }

  // * Copy Constructor
  Webserv(const Webserv &other) { (void)other; }

  // * Copy assignment operator
  Webserv &operator=(const Webserv &other) {
    (void)other;
    return *this;
  }

  // * Destructor
  ~Webserv() {}

  // * ENUM
  enum METHOD {
    GET,
    POST,
    DELETE,
  };

  enum STATUS_CODE {
    // * 2xx Success
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,

    // * 3xx Redirection
    MOVED_PERMANENTLY = 301,
    FOUND = 302,

    // * 4xx Client Error
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    PAYLOAD_TOO_LARGE = 413,

    // * 5xx Server Error
    INTERNAL_SERVER_ERROR = 500,
    BAD_GATEWAY = 502,
    GATEWAY_TIMEOUT = 504
  };

  // * Methods
  void initMimeTypes();
};

// ! Server
class Server : public Webserv {
  // ! private
private:
  int sockfd;
  // ! public
public:
  // * Default Constructor
  Server() : sockfd(-1) {}

  // * Copy Constructor
  Server(const Server &other) : Webserv(other), sockfd(-1) {}

  // * Copy assignment operator
  Server &operator=(const Server &other) {
    if (this != &other) {
      this->sockfd = other.sockfd;
    }
    return *this;
  }

  // * Destructor
  ~Server() {};

  // * Getters & Setters
  int getSockFd() const { return this->sockfd; };
  void setSockFd(int value) { this->sockfd = value; };

  // * Methods
  void run();
};

// ! REQUEST
class Request : public Webserv {
  // ! private
private:
  // ! public
public:
  METHOD method;
  std::string path;
  std::string httpV;

  // * Default Constructor
  Request() {}

  // * Copy Constructor
  Request(const Request &other) : Webserv(other) {}

  // * Copy assignment operator
  Request &operator=(const Request &other) {
    if (this != &other) {
      // logic
    }
    return *this;
  }

  // * Destructor
  ~Request() {};

  // * Getters & Setters
  void setRequest(const std::string &req);
  const std::map<std::string, std::string> &getRequest() const;

  // * Methods
};

// ! RESPONSE
class Response : public Webserv {
  // ! private
private:
  std::string res;
  STATUS_CODE status_code;
  std::string statusLine;
  std::string contentType;
  std::string contentLength;
  std::string headers;
  static const std::string serverName;
  std::string body;

  // ! public
public:
  // * Default Constructor
  // Response()
  //     : res(""), status_code(Webserv::NO_CONTENT), statusLine(""),
  //     headers(""),
  //       body("") {}

  // * Copy Constructor
  // Response(const Response &other)
  //     : Webserv(other), statusCode(other.statusCode), headers(other.headers),
  //       body(other.body) {}

  // * Copy assignment operator
  Response &operator=(const Response &other) {
    if (this != &other) {
      this->status_code = other.status_code;
      this->headers = other.headers;
      this->body = other.body;
    }
    return *this;
  }

  // * Destructor
  ~Response() {};

  // * Getters & Setters
  void setStatusCode(STATUS_CODE value) { this->status_code = value; }
  STATUS_CODE getStatusCode() const { return this->status_code; }

  void setStatusLine(const std::string httpV,
                     const std::string &statusCodeDescription) {
    this->statusLine = httpV + " " + statusCodeDescription + "\r\n";
  }
  std::string getStatusLine() const { return this->statusLine; }

  void setContentType(const std::string &path);
  std::string getContentType() const { return this->contentType; }

  void setContentLength(const std::string &body) {
    size_t len = body.length();
    std::stringstream ss;
    ss << len;
    this->contentLength = "Content-Length: " + ss.str() + "\r\n";
  };
  std::string getContentLength() const { return this->contentLength; }

  std::string getHeaders() const { return this->headers; }
  void setHeaders() {
    this->headers = this->getStatusLine() + this->serverName +
                    this->getContentType() + getContentLength() + "\r\n";
  }

  void setBody(std::ifstream &file) {
    std::string line;
    this->body.clear();
    while (std::getline(file, line)) {
      this->body += line + "\n";
    }
    this->body += "\n\r";
  }
  std::string getBody() const { return this->body + "\r\n"; }

  void setResponse() { this->res = getHeaders() + getBody(); }
  std::string getResponse() const { return this->res; }

  // * Methods
  void GET_METHOD(Request &req);
  void POST_METHOD();
  void DELETE_METHOD();
  std::string statusCodeDescription(STATUS_CODE statusCode);
  void response(const int clientFd, Request &req);
};

// * Prototype of functions

#endif