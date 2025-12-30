/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/28 10:07:08 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP
# include <climits>
# include <ctime>
# include <fcntl.h>
# include <fstream>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <sstream>
# include <cstring>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <unistd.h>
# define PORT 8080
# define IP INADDR_ANY
# define IPv4 AF_INET
# define BACK_LOG 4096
# define MAX_BUFFER_SIZE 4096
# define MAX_EVENTS 1024


// ****************************************************************************** //
//                                  Webserv Class                                 //
// ****************************************************************************** //

class Webserv {

	public:
		std::map<std::string, std::string>	request;
		std::map<std::string, std::string>	mimeTypes;
		// std::map<std::string, std::string> response;

		Webserv();
		Webserv(const Webserv &other);
		Webserv	&operator=(const Webserv &other);
		~Webserv();

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

		// member functions
		void	initMimeTypes();
		void	throwError(std::string func);

};


// ****************************************************************************** //
//                                  Server Class                                  //
// ****************************************************************************** //

class Server : public Webserv {

	private:
		int	_sockfd;

	public:
		Server();

		int		getSockFd() const;
		void	setSockFd(int fd);
		void	run();

};


// ****************************************************************************** //
//                                 Request Class                                  //
// ****************************************************************************** //

class Request : public Webserv {

	public:
		METHOD		method;
		std::string	path;
		std::string	httpV;

		void	setRequest(const std::string &req);
		const	std::map<std::string, std::string> &getRequest() const;

};


// ****************************************************************************** //
//                                 Response Class                                 //
// ****************************************************************************** //

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