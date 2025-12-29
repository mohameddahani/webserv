/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/29 17:23:09 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// * initialize server name
const std::string Response::serverName = "webserv/1.0\r\n";

// * Default Constructor
Response::Response() { this->setMimeTypes(); }

// * Getters & Setters
// * status code
void Response::setStatusCode(STATUS_CODE value) { this->status_code = value; }

Webserv::STATUS_CODE Response::getStatusCode() const {
  return this->status_code;
}

void Response::setStatusLine(const std::string httpV,
                             const std::string &statusCodeDescription) {
  this->statusLine = httpV + " " + statusCodeDescription + "\r\n";
}
std::string Response::getStatusLine() const { return this->statusLine; }

// * content type
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

  std::map<std::string, std::string>::const_iterator it =
      this->getMimeTypes().find(extention);

  if (it == this->getMimeTypes().end()) {
    this->contentType =
        "Content-Type: application/octet-stream\r\n"; // * we dont know what
    // * is the type of
    // * content
    return;
  }

  this->contentType = "Content-Type: " + it->second + "\r\n";
}

std::string Response::getContentType() const { return this->contentType; }

// * content length
void Response::setContentLength(const std::string &body) {
  size_t len = body.length();
  std::stringstream ss;
  ss << len;
  this->contentLength = "Content-Length: " + ss.str() + "\r\n";
};

std::string Response::getContentLength() const { return this->contentLength; }

// * headers
std::string Response::getHeaders() const { return this->headers; }

void Response::setHeaders() {
  this->headers = this->getStatusLine() + this->serverName +
                  this->getContentType() + getContentLength() + "\r\n";
}

// * body
void Response::setBody(std::ifstream &file) {
  std::string line;
  this->body.clear();
  while (std::getline(file, line)) {
    this->body += line + "\n";
  }
  this->body += "\n\r";
}
std::string Response::getBody() const { return this->body + "\r\n"; }
void Response::addDataToBody(size_t pos, std::string &data) {
  this->body.insert(pos, data);
}

// * response
void Response::setResponse() { this->res = getHeaders() + getBody(); }
std::string Response::getResponse() const { return this->res; }

// ! Order of response
// HTTP/1.1 200 OK
// Server: webserv/1.0
// Content-Type: text/html
// Content-Length: 128
// Connection: close

// <body>

// * GET METHOD
void Response::GET_METHOD(const Request &req) {
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

  // * Generate response
  this->generateResponse(req, fullPath);
}

// * POST METHOD
void Response::POST_METHOD(const Request &req) {
  std::string pathOfDataForm = "pages/post-request-data.html";
  std::string pathOfDataUploads = "uploads";

  // * check content type
  std::string postContentType =
      req.getRequest().count("Content-Type")
          ? req.getRequest().find("Content-Type")->second
          : "";

  // * handle all content type
  // ? html form this is not plain human text like HTML
  // ? (email=mdahani%40student.1337.ma&password=1337)
  if (postContentType == "application/x-www-form-urlencoded") {
    // * check the file permissions and if the file exist
    if (access(pathOfDataForm.c_str(), F_OK) == -1) {
      this->setStatusCode(this->NOT_FOUND);
      pathOfDataForm = "pages/errors/404.html";
    } else if (access(pathOfDataForm.c_str(), R_OK) == -1) {
      this->setStatusCode(this->FORBIDDEN);
      pathOfDataForm = "pages/errors/403.html";
    } else {
      this->setStatusCode(this->OK);
    }
  } else if (postContentType == "multipart/form-data") { // ? (img, video, ...)
  }
  // * Generate response
  this->generateResponse(req, pathOfDataForm);
}

// * DELETE METHOD
void Response::DELETE_METHOD(const Request &req) { (void)req; }

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

// * parse form URL encoded
std::map<std::string, std::string>
Response::parseFormURLEncoded(const std::string &post_body) {
  std::map<std::string, std::string> result;

  std::stringstream ss(post_body);
  std::string line;

  while (std::getline(ss, line, '&')) {
    size_t pos = line.find("=");

    if (pos == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    result[key] = value;
  }

  return result;
}

// * Method Not Allowed
void Response::methodNotAllowed(const Request &req) {
  // * set status code
  this->setStatusCode(METHOD_NOT_ALLOWED);

  // * full path
  std::string fullPath = "/pages/errors/405.html";

  // * Generate response
  this->generateResponse(req, fullPath);
}

// * Generate response
void Response::generateResponse(const Request &req, std::string &path) {
  // * status line
  this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));

  // * open file
  std::ifstream file(path.c_str());
  if (!file.is_open()) {
    // todo: show error in browser
    std::cerr << "Error: file is not open !" << std::endl;
    file.close();
    return;
  }

  // * Content Type
  this->setContentType(path);

  // * add the data of post request to body
  // todo: add data in post-request-data.html

  // * Body
  this->setBody(file);
  // ? body of post method
  if (req.method == POST) {
    // * get content type to decide which response will send in post method
    // std::string contentType =
    //     req.getRequest().count("Content-Type")
    //         ? req.getRequest().find("Content-Type")->second
    //         : "";
    // if (contentType ==) {
    //   /* code */
    // }

    // ? add raw data if method is post and content type is
    // ? application/x-www-form-urlencoded
    size_t pos = this->getBody().find("<!-- raw data -->\n");
    if (pos != std::string::npos) {
      // * get post body
      std::string post_body =
          req.getRequest().count("post-body")
              ? req.getRequest().find("post-body")->second + "\n"
              : "";

      // * add raw data to html file
      this->addDataToBody(pos + strlen("<!-- raw data -->\n"), post_body);

      // * add parsed data to file html
      std::map<std::string, std::string> parsedData =
          this->parseFormURLEncoded(post_body);

      pos = this->getBody().find("<!-- parsed data -->\n");
      if (pos != std::string::npos) {
        // * skip the comment of html
        pos += strlen("<!-- parsed data -->\n");
        // * merge data with html and send it
        std::map<std::string, std::string>::iterator itParsedData =
            parsedData.begin();
        for (; itParsedData != parsedData.end(); ++itParsedData) {
          post_body.clear();
          post_body.clear();

          post_body = "<div class=\"flex justify-between bg-zinc-800 px-4 py-2 "
                      "rounded\">\n"
                      "<span class=\"text-blue-400 font-medium\">" +
                      itParsedData->first + "</span>\n";

          post_body += "<span class=\"text-white\">" + itParsedData->second +
                       "</span>\n"
                       "</div>\n";

          this->addDataToBody(pos, post_body);
          // * update position
          pos += post_body.length();
        }
      }
    }
  }

  // * Content Length
  this->setContentLength(this->getBody());

  // * merge all headers
  this->setHeaders();

  // * create response
  this->setResponse();

  // ! close the file
  file.close();
}

// * Response
void Response::response(const Request &req) {
  if (req.method == GET) {
    this->GET_METHOD(req);
  } else if (req.method == POST) {
    this->POST_METHOD(req);
  } else if (req.method == DELETE) {
    this->DELETE_METHOD(req);
  } else {
    this->methodNotAllowed(req);
  }
}
