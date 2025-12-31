/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/31 12:08:12 by mdahani          ###   ########.fr       */
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

// ******************************************************************************
// //
//                                  Webserv Class //
// ******************************************************************************
// //

class Webserv {

  public:
    // * ENUM
    enum METHOD {
      GET,
      POST,
      DELETE,
      ELSE,
    };

    enum STATUS_CODE {
      // * 2xx Success
      OK = 200,
      CREATED = 201,
      NO_CONTENT = 204,

      // * 3xx Redirection
      MOVED_PERMANENTLY = 301,
      FOUND = 302,

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

      // * 5xx Server Error
      INTERNAL_SERVER_ERROR = 500,
      BAD_GATEWAY = 502,
      GATEWAY_TIMEOUT = 504
    };
};

// ******************************************************************************
// //
//                                  Server Class //
// ******************************************************************************
// //

class Server : public Webserv {

  private:
    int _sockfd;

	public:
		Server();

		int		getSockFd() const;
		void	setSockFd(int fd);
		void	setNonBlocking(int fd);
		void	run();

    int getSockFd() const;
    void setSockFd(int fd);
    void run();
};

// ******************************************************************************
// //
//                                 Request Class //
// ******************************************************************************
// //

class Request : public Webserv {

  private:
    std::map<std::string, std::string> request;

  public:
    METHOD method;
    std::string path;
    std::string httpV;

    void setRequest(const std::string &req);
    const std::map<std::string, std::string> &getRequest() const;
};

// ******************************************************************************
// //
//                                 Response Class //
// ******************************************************************************
// //

class Response : public Webserv {
  private:
    std::map<std::string, std::string> mimeTypes;
    std::string res;
    STATUS_CODE status_code;
    std::string statusLine;
    std::string contentType;
    std::string contentLength;
    std::string headers;
    static const std::string serverName;
    std::string body;

  public:
    // * Default Constructor
    Response();

    // * Getters & Setters
    void setMimeTypes();
    const std::map<std::string, std::string> &getMimeTypes() const;

    void setStatusCode(STATUS_CODE value);
    STATUS_CODE getStatusCode() const;

    void setStatusLine(const std::string httpV,
                       const std::string &statusCodeDescription);
    std::string getStatusLine() const;

    void setContentType(const std::string &path);
    std::string getContentType() const;

    void setContentLength(const std::string &body);
    std::string getContentLength() const;

    std::string getHeaders() const;
    void setHeaders();

    void setBody(std::ifstream &file);
    std::string getBody() const;
    void addDataToBody(size_t pos, std::string &data);

    void setResponse();
    std::string getResponse() const;

    // * Methods
    void GET_METHOD(const Request &req);
    void POST_METHOD(const Request &req);
    void DELETE_METHOD(const Request &req);
    std::string statusCodeDescription(STATUS_CODE statusCode);
    std::map<std::string, std::string>
    parseFormURLEncoded(const std::string &post_body);
    void generateResponse(const Request &req, std::string &path);
    void methodNotAllowed(const Request &req);
    void response(const Request &req);
};

#endif