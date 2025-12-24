/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/24 21:00:06 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// * includes
#include <climits>
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
  // std::map<std::string, std::string> response;

  // * Default Constructor
  Webserv() {}

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

  // * Methods
  void parseRequest(std::string &req);
};

// ! RESPONSE
class Response : public Webserv {
  // ! private
private:
  unsigned int statusCode;
  std::string headers;
  std::string body;

  // ! public
public:
  // * Default Constructor
  Response() : statusCode(0), headers(""), body("") {}

  // * Copy Constructor
  Response(const Response &other)
      : Webserv(other), statusCode(other.statusCode), headers(other.headers),
        body(other.body) {}

  // * Copy assignment operator
  Response &operator=(const Response &other) {
    if (this != &other) {
      this->statusCode = other.statusCode;
      this->headers = other.headers;
      this->body = other.body;
    }
    return *this;
  }

  // * Destructor
  ~Response() {};

  // * Getters & Setters
  unsigned int getStatusCode() const { return this->statusCode; }
  void setStatusCode(unsigned int value) { this->statusCode = value; }

  std::string getHeaders() const { return this->headers; }
  void setHeaders(std::string value) { this->headers = value; }

  std::string getBody() const { return this->body; }
  void setBody(std::string value) { this->body = value; }

  // * Methods
  void GET_METHOD();
  void POST_METHOD();
  void DELETE_METHOD();
};

// * Prototype of functions
void response(int clientFd, std::string req);

#endif