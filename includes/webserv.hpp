/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/19 20:57:51 by mdahani          ###   ########.fr       */
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
};

// ! RESPONSE
class Response {
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
  // size_t getLengthOfBody() {}
};

// * Prototype of functions
void response(int clientFd);

#endif