/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2026/01/07 17:09:06 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#define PORT 8080
#define IP INADDR_ANY
#define IPv4 AF_INET
#define BACK_LOG 4096
#define MAX_BUFFER_SIZE 4096
#define MAX_EVENTS 1024

// ****************************************************************************** //
//                                  Webserv Class                                 //
// ****************************************************************************** //

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
    void throwError(std::string func);
};

// ****************************************************************************** //
//                                  Server Class                                  //
// ****************************************************************************** //

class Request;

typedef struct s_clientState {
	int			fd;
    std::string	request;
    size_t		bytes_received;
    size_t		content_length;
    bool		isPostRequest;
    bool		headers_complete;
    bool		request_complete;
    bool		isHeaderSent;

	s_clientState(): fd(-1), bytes_received(0), content_length(0), isPostRequest(false), headers_complete(false), request_complete(false), isHeaderSent(false) {};

} t_clientState;

class Server : public Webserv {
	private:
		int								_sockfd;
		int								_epollfd;
		int								_port;
		std::string						_host;
		struct sockaddr_in				_serverAddr;

	public:
		std::map<int, t_clientState>	clients;

		Server();
		Server(std::string& host, int port);

		int		getSockFd() const;
		void	setSockFd(int fd);
		void	setNonBlocking(int fd);
		bool	isCompleteRequest(std::string &request);
		size_t	getContentLength(std::string &request);
		void	setUpNewConnection(int epfd, int serverFd, epoll_event& ev);
		bool	recvRequest(int epfd, int notifiedFd, epoll_event ev);
		bool	sendResponse(int epfd, int notifiedFd, Request &request);
		void	initServerAddress();
		void	createServerSocket();
		void	bindServerSocket();
		void	startListening();
		void	createEpollInstance();
		void	addServerToEpoll();
		void	handleEpollEvents(int nfds, struct epoll_event* events, Request& req);
		void	processClientEvent(struct epoll_event& event, Request& req);
		void	processServerEvent(struct epoll_event& ev);
		void	run(Request &req);
};


// ****************************************************************************** //
//                               ConfigFile Class                                 //
// ****************************************************************************** //

typedef struct location
{
    std::string					path;
    std::vector<std::string>	allow_methods;
    bool						autoindex;
    std::string					root;
    std::string					return_to;
    std::string					index;
} location;

class ConfigFile {
	public:
		std::vector<int> listen;
		std::string server_name;
		std::string host;
		std::string root;
		int client_max_body_size;
		std::string index;
		std::map<int, std::string> error_page;
		std::vector<location> locations;
		std::vector<std::string> cgi_path;
		std::vector<std::string> cgi_ext;

		void  init_the_header_conf_default();
    	void  parse_config_file(char *av);
};


// ****************************************************************************** //
//                                 Request Class                                  //
// ****************************************************************************** //

class Request : public Webserv {

  private:
    std::map<std::string, std::string>	request;

  public:
    METHOD		method;
    std::string	path;
    std::string	httpV;
	  ConfigFile	config;

    void	setRequest(const std::string &req);
    const	std::map<std::string, std::string> &getRequest() const;
};


// ****************************************************************************** //
//                                 Response Class                                 //
// ****************************************************************************** //

class Response : public Webserv {
  private:
    std::map<std::string, std::string> mimeTypes;
    STATUS_CODE status_code;
    std::string statusLine;
    std::string contentType;
    std::string contentLength;
    std::string headers;
    size_t indexLocation;
    int bodyFd;

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

    std::string getServerName(const Request &req) const;

    void setContentType(const std::string &path);
    std::string getContentType() const;

    void setContentLength(const size_t &bodyLength);
    std::string getContentLength() const;

    std::string getHeaders() const;
    void setHeaders(const Request &req);
  
    size_t getIndexLocation() const;
    void setIndexLocation(size_t &value);

    void setBodyFd(int &fd);
    int getBodyFd() const;

    // * Methods
    void GET_METHOD(Request &req);
    void POST_METHOD(Request &req);
    void DELETE_METHOD(Request &req);
    std::string statusCodeDescription(STATUS_CODE statusCode);
    size_t countBodyLength(const std::string &path);
    void addDataToBody(const Request &req);
    std::map<std::string, std::string>
    parseFormURLEncoded(const std::string &post_body);
    bool thisLocationIsInConfigFile(Request &req, std::string &location);
    bool checkAllowMethodsOfLocation(std::vector<std::string> &allowMethods, std::string method);
    std::string generatePageOfAutoIndex(Request &req, std::string &pathOfAutoIndex);
    bool isPathStartBySlash(const std::string &path);
    void generateResponse(Request &req);
    void methodNotAllowed(Request &req);
    void response(Request &req);
};

#endif