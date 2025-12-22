/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/21 11:12:41 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// * includes
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

// * classes
class Webserv {
  // ! private
private:
  // ! public
public:
  // * Default Constructor
  Webserv() {}

  // * Copy Constructor
  Webserv(const Webserv &other) {}

  // * Copy assignment operator
  Webserv &operator=(const Webserv &other) { return *this; }

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

// ! RESPONSE
class Response: public Webserv {
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
      : statusCode(other.statusCode), headers(other.headers), body(other.body) {
  }

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