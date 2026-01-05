/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2026/01/05 15:49:19 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

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

// * server name
std::string Response::getServerName(const Request &req) const {
  return "Server: " + req.server_name + "\r\n";
}

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
void Response::setContentLength(const size_t &bodyLength) {
  std::stringstream ss;
  ss << bodyLength;
  this->contentLength = "Content-Length: " + ss.str() + "\r\n";
};

std::string Response::getContentLength() const { return this->contentLength; }

// * headers
std::string Response::getHeaders() const { return this->headers; }

void Response::setHeaders(const Request &req) {
  this->headers = this->getStatusLine() + getServerName(req) +
                  this->getContentType() + getContentLength() + "\r\n";
}

void Response::setBodyFd(int &fd) { this->bodyFd = fd; }
int Response::getBodyFd() const { return this->bodyFd; }

// ! Order of response
// HTTP/1.1 200 OK
// Server: webserv/1.0
// Content-Type: text/html
// Content-Length: 128
// Connection: close

// <body>

// * GET METHOD
void Response::GET_METHOD(Request &req) {
  if (req.path == "/") {
    req.path.append(req.index);
  }

  // * set status code as default
  this->setStatusCode(OK);

  // * Generate response
  this->generateResponse(req);
}

// * POST METHOD
void Response::POST_METHOD(Request &req) {
  // * set status code as default
  this->setStatusCode(CREATED);

  // * get content type to decide which response will send in post method
  std::string contentType = req.getRequest().count("Content-Type")
                                ? req.getRequest().find("Content-Type")->second
                                : "";

  // ? application/x-www-form-urlencoded
  if (contentType == "application/x-www-form-urlencoded\r") {
    // * set all data in html page
    this->addDataToBody(req);
    req.path = "/post-request-data.html";
  } else if (contentType.substr(0, 52) == // ? multipart/form-data;
                                          // ? boundary=----WebKitFormBoundary
             "multipart/form-data; boundary=----WebKitFormBoundary") {
    std::string uploadBody = req.getRequest().count("binary-data")
                                 ? req.getRequest().find("binary-data")->second
                                 : "";
    // * check if the file is empty
    if (uploadBody.empty()) {
      // todo: i think i should make this path flexible (get from config file)
      req.path = "/post-request-error-upload.html";
      this->setStatusCode(NO_CONTENT);
    } else if (uploadBody.length() / 1e6 >
               req.client_max_body_size) { // * check file size by config file
                                           // * convert from bytes to MB
      this->setStatusCode(FORBIDDEN);
      // todo: i think i should make this path flexible (get from config file)
      req.path = "/errors/403.html";
    } else {
      std::string filename = req.getRequest().count("filename")
                                 ? req.getRequest().find("filename")->second
                                 : "";
      if (filename.empty()) {
        std::cerr << "Error: There no name on file!" << std::endl;
        return;
      }

      std::string fullPath = "uploads/";

      // * check the directory of upload
      std::ifstream uploadDir(fullPath.c_str());
      if (!uploadDir.is_open()) {
        this->setStatusCode(FORBIDDEN);
        // todo: i think i should make this path flexible (get from config file)
        req.path = "/errors/403.html";
      } else {
        fullPath.append(filename);

        std::ofstream outputFile(fullPath.c_str(),
                                 std::ios::binary | std::ios::out);

        if (!outputFile.is_open()) {
          std::cerr << "Error: Failed to create file!" << std::endl;
          uploadDir.close();
          return;
        }

        outputFile << uploadBody;

        // ! close output file
        uploadDir.close();
        outputFile.close();

        // todo: i think i should make this path flexible (get from config file)
        req.path = "/post-request-upload.html";
      }
    }
  }

  // * Generate response
  this->generateResponse(req);
}

// * DELETE METHOD
void Response::DELETE_METHOD(const Request &req) { (void)req; }

// * Status code description
std::string Response::statusCodeDescription(STATUS_CODE statusCode) {
  if (statusCode == OK) {
    return "200 OK";
  } else if (statusCode == CREATED) {
    return "201 Created";
  } else if (statusCode == NO_CONTENT) {
    return "204 No Content";
  } else if (statusCode == MOVED_PERMANENTLY) {
    return "301 Moved Permanently";
  } else if (statusCode == FOUND) {
    return "302 Found";
  } else if (statusCode == BAD_REQUEST) {
    return "400 Bad Request";
  } else if (statusCode == FORBIDDEN) {
    return "403 Forbidden";
  } else if (statusCode == NOT_FOUND) {
    return "404 Not Found";
  } else if (statusCode == METHOD_NOT_ALLOWED) {
    return "405 Method Not Allowed";
  } else if (statusCode == PAYLOAD_TOO_LARGE) {
    return "413 Payload Too Large";
  } else if (statusCode == INTERNAL_SERVER_ERROR) {
    return "500 Internal Server Error";
  } else if (statusCode == BAD_GATEWAY) {
    return "502 Bad Gateway";
  } else if (statusCode == GATEWAY_TIMEOUT) {
    return "504 Gateway Timeout";
  }
  return "Unknown Status";
}

// * count the body length
size_t Response::countBodyLength(const std::string &path) {
  struct stat buffer;

  if (stat(path.c_str(), &buffer) == -1) {
    return 0;
  }

  return buffer.st_size;
}

// * add data to body
void Response::addDataToBody(const Request &req) {
  std::string beforRawData =
      "<!DOCTYPE html>\n"
      "<html lang=\"en\">\n"
      "<head>\n"
      "  <meta charset=\"UTF-8\" />\n"
      "  <meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1.0\" />\n"
      "  <script "
      "src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></script>\n"
      "  <title>POST Request Data</title>\n"
      "</head>\n"
      "\n"
      "<body class=\"min-h-screen bg-black flex items-center "
      "justify-center\">\n"
      "  <!-- Card -->\n"
      "  <div class=\"bg-zinc-900 border border-zinc-700 rounded-xl shadow-lg "
      "p-6 w-full max-w-xl\">\n"
      "\n"
      "    <!-- Title -->\n"
      "    <h1 class=\"text-xl font-bold text-white mb-4 text-center\">\n"
      "      📩 POST Request Body\n"
      "    </h1>\n"
      "\n"
      "    <div class=\"mb-4\">\n"
      "      <p class=\"text-sm text-zinc-400 mb-1\">\n"
      "        Raw data :\n"
      "      </p>\n"
      "\n"
      "<!-- raw data -->\n"
      "      <pre class=\"bg-black text-green-400 p-3 rounded text-sm "
      "overflow-x-auto\">\n";

  std::string beforParseData =
      "        </pre>\n"
      "  </div>\n"
      "  <div>\n"
      "    <p class=\"text-sm text-zinc-400 mb-2 mt-4\">\n"
      "      Parsed fields :\n"
      "    </p>\n"
      "    <div class=\"space-y-2\">\n"
      "      <!-- parsed data -->\n";

  std::string afterPaseData = "    </div>\n"
                              "  </div>\n"
                              "</div>\n"
                              "</body>\n"
                              "</html>\n";

  // todo: get the root path
  std::string fullPath("pages");

  fullPath.append(req.path);

  // ? std::ios::out open the file for write event and if not exist created
  // ? std::ios::trunc remove all things in file
  std::ofstream file(fullPath.c_str(), std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    // todo: print error in terminal
    std::cerr << "file is not open(addDataToBoddy)" << std::endl;
    return;
  }

  // * insert all data in file
  file << beforRawData;

  // * get post-body
  std::string body_post = req.getRequest().count("post-body")
                              ? req.getRequest().find("post-body")->second
                              : "";

  file << body_post;

  file << beforParseData;

  // * parse body post
  std::map<std::string, std::string> dataOfBody =
      this->parseFormURLEncoded(body_post);

  std::map<std::string, std::string>::iterator it = dataOfBody.begin();
  std::string parseData;
  for (; it != dataOfBody.end(); ++it) {

    parseData +=
        "<div class=\"flex justify-between bg-zinc-800 px-4 py-2 rounded\">\n"
        "<span class=\"text-blue-400 font-medium\">" +
        it->first +
        "</span>"
        "<span class=\"text-white\">" +
        it->second +
        "</span>"
        "</div>\n";
  }

  // * add raw data
  file << parseData;

  file << afterPaseData;
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
void Response::methodNotAllowed(Request &req) {
  // * set status code
  this->setStatusCode(METHOD_NOT_ALLOWED);

  // * full path
  std::string fullPath = "pages/errors/405.html";

  // * Generate response
  this->generateResponse(req);
}

// * Generate response
void Response::generateResponse(Request &req) {
  // * root directory
  // todo: check if we have the folder
  std::string fullPath(req.root);

  // * add path to root directory
  fullPath.append(req.path);

  // * check the file permissions and if the file exist
  // todo: change the path of error pages by config file and check if we have
  // todo: the folder
  if (access(fullPath.c_str(), F_OK) == -1) {
    this->setStatusCode(NOT_FOUND);
    fullPath = (req.error_page[NOT_FOUND]);
    // fullPath = "pages/errors/404.html";
  } else if (access(fullPath.c_str(), R_OK) == -1 ||
             access(fullPath.c_str(), W_OK) == -1) {
    this->setStatusCode(FORBIDDEN);
    fullPath = (req.error_page[FORBIDDEN]);
    // fullPath = "pages/errors/403.html";
  }

  // * status line
  this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));

  // * Content Type
  this->setContentType(fullPath);

  // * Content Length
  this->setContentLength(this->countBodyLength(fullPath));

  // * merge all headers
  this->setHeaders(req);

  // * get fd of body
  int fd = open(fullPath.c_str(), O_RDONLY);
  this->setBodyFd(fd);

  std::cout << "-----------------------path of file------------------------"
            << std::endl;
  std::cout << fullPath << std::endl;
  std::cout << "-----------------------path of file------------------------"
            << std::endl;

  std::cout << "-----------------------Headers------------------------"
            << std::endl;
  std::cout << getHeaders() << std::endl;
  std::cout << "-----------------------Headers------------------------"
            << std::endl;
}

// * Response
void Response::response(Request &req) {
  if (req.method == GET) {
    std::cout << "=======GET=======" << std::endl;
    this->GET_METHOD(req);
  } else if (req.method == POST) {
    std::cout << "=======POST=======" << std::endl;
    this->POST_METHOD(req);
  } else if (req.method == DELETE) {
    std::cout << "=======DELETE=======" << std::endl;
    this->DELETE_METHOD(req);
  } else {
    this->methodNotAllowed(req);
    std::cout << "=======ELSE=======" << std::endl;
  }
}
