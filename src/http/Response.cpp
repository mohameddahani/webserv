/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 10:45:08 by mdahani           #+#    #+#             */
/*   Updated: 2026/02/25 11:35:00 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/Response.hpp"

// Default Constructor
Response::Response()
    : status_code(OK), statusLine(""), contentType(""), contentLength(""),
      headers(""), indexLocation(-1), isRedirection(false), bodyFd(-1) {
  this->setMimeTypes();
}

// Getters & Setters
void Response::setStatusCode(STATUS_CODE value) { this->status_code = value; }

Webserv::STATUS_CODE Response::getStatusCode() const {
  return this->status_code;
}

void Response::setStatusLine(const std::string httpV,
                             const std::string &statusCodeDescription) {
  this->statusLine = httpV + " " + statusCodeDescription + "\r\n";
}
std::string Response::getStatusLine() const { return this->statusLine; }

// server name
std::string Response::getServerName(const Request &req) const {
  return "Server: " + req.config.server_name + "\r\n";
}

// content type
void Response::setContentType(const std::string &path) {
  size_t pos = path.rfind(".");

  if (pos == std::string::npos) {
    this->contentType =
        "Content-Type: application/octet-stream\r\n"; // we don't know what is the type of content

    return;
  }

  std::string extention = path.substr(pos);

  std::map<std::string, std::string>::const_iterator it =
      this->getMimeTypes().find(extention);

  if (it == this->getMimeTypes().end()) {
    this->contentType =
        "Content-Type: application/octet-stream\r\n"; // we don't know what is the type of content

    return;
  }

  this->contentType = "Content-Type: " + it->second + "\r\n";
}

std::string Response::getContentType() const { return this->contentType; }

// content length
void Response::setContentLength(const size_t &bodyLength) {
  std::stringstream ss;
  ss << bodyLength;
  this->contentLength = "Content-Length: " + ss.str() + "\r\n";
};

std::string Response::getContentLength() const { return this->contentLength; }

// headers
std::string Response::getHeaders() const { return this->headers; }

void Response::setHeaders(const Request &req) {
  // get content type to decide which headers will send it
  std::string contentType = req.getRequest().count("Content-Type")
                                ? req.getRequest().find("Content-Type")->second
                                : "";

  std::string sessionIdFromBrowser = this->parseFormURLEncoded(
      req.getRequest().count("Cookie") ? req.getRequest().find("Cookie")->second
                                       : "")["session_id"];
  std::string sessionIdFromServer =
      req.getSession().count("session_id")
          ? req.getSession().find("session_id")->second
          : "";

  // check if is redirection
  if (this->getIsRedirection() && this->getStatusCode() == FOUND) {
    this->headers = this->getStatusLine() + getServerName(req) + "Location: " +
                    req.config.locations[this->getIndexLocation()].return_to +
                    "\r\n";
    this->setIsRedirection(false);
  } else if (req.method == DELETE && this->getStatusCode() == NO_CONTENT) {
    // this for delete response
    this->headers = this->getStatusLine() + getServerName(req) + "\r\n";
  } else if (contentType == "application/x-www-form-urlencoded\r" &&
             req.method == POST) {
	 // this for send Cookies headers	
    this->headers = this->getStatusLine() + getServerName(req) +
                    this->getContentType() +
                    "Set-Cookie: session_id=" + sessionIdFromServer + "\r\n" +
                    getContentLength() + "\r\n";
  }

  else if (!this->isUserInSession(req, sessionIdFromBrowser) &&
           this->getStatusCode() == FOUND) {
	// check auth by Cookies
    this->headers = this->getStatusLine() + getServerName(req) +
                    "Location: sign-in.html\r\n";
  }

  else if (this->isUserInSession(req, sessionIdFromBrowser) &&
           this->getStatusCode() == FOUND) {
	// check auth by Cookies
    this->headers =
        this->getStatusLine() + getServerName(req) + "Location: game.html\r\n";
  }

  else {
    // this for normal response
    this->headers = this->getStatusLine() + getServerName(req) +
                    this->getContentType() + getContentLength() + "\r\n";
  }
}

// index location
size_t Response::getIndexLocation() const { return this->indexLocation; }

void Response::setIndexLocation(size_t &value) { this->indexLocation = value; }

// is redirection
bool Response::getIsRedirection() const { return this->isRedirection; }

void Response::setIsRedirection(bool value) { this->isRedirection = value; }

// Fd of Body
void Response::setBodyFd(int &fd) { this->bodyFd = fd; }
int Response::getBodyFd() const { return this->bodyFd; }


// GET METHOD
void Response::GET_METHOD(Request &req) {
  // set status code as default
  this->setStatusCode(OK);

  // check rules if location is in config file
  if (!req.config.locations.empty()) {
    if (this->thisLocationIsInConfigFile(req, req.path)) {
      // redirection
      if (!req.config.locations[this->getIndexLocation()].return_to.empty()) {
        // Generate response (only headers)
        this->setIsRedirection(true);
        this->setStatusCode(FOUND);
        this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));
        this->setHeaders(req);

        return;
      }
      // check method
      if (this->checkAllowMethodsOfLocation(
              req.config.locations[this->getIndexLocation()].allow_methods,
              "get")) {
        // check if is file or directory
        bool isFile =
            this->isFile(req.config.locations[this->getIndexLocation()].root +
                         req.config.locations[this->getIndexLocation()].path);
        // generate page of auto index
        if (req.config.locations[this->getIndexLocation()].autoindex) {
          // check if the path is file because auto index is work only on
          // folder
          if (isFile) {
            this->setStatusCode(FORBIDDEN);
            req.path = req.config.error_page[FORBIDDEN];
          } else {
            std::string pathOfAutoIndex =
                req.config.locations[this->getIndexLocation()].root +
                req.config.locations[this->getIndexLocation()].path;
            req.path = this->generatePageOfAutoIndex(req, pathOfAutoIndex);
          }
        }
        // check if autoindex is off and we dont have a index html
        else if (!req.config.locations[this->getIndexLocation()].autoindex &&
                 req.config.locations[this->getIndexLocation()].index.empty() &&
                 !isFile) {
          this->setStatusCode(FORBIDDEN);
          req.path = req.config.error_page[FORBIDDEN];

        } else {
          if (isFile &&
              !req.config.locations[this->getIndexLocation()].index.empty()) {
            this->setStatusCode(FORBIDDEN);
            req.path = req.config.error_page[FORBIDDEN];
          } else {
            // change root path from config file when i found location and
            // method
            req.config.root =
                req.config.locations[this->getIndexLocation()].root;
            // change path from config file when i found location and method
            req.path = req.config.locations[this->getIndexLocation()].path +
                       (isFile ? "" : "/") +
                       req.config.locations[this->getIndexLocation()].index;
          }
        }
      } else {
        this->setStatusCode(METHOD_NOT_ALLOWED);
        req.path = req.config.error_page[METHOD_NOT_ALLOWED];
      }
    }
  }

  if (req.path == "/") {
    req.path = req.config.index;
  }

  // check auth cookies of user
  if (req.path == "/game.html") {
    std::string sessionIdFromBrowser =
        this->parseFormURLEncoded(req.getRequest().count("Cookie")
                                      ? req.getRequest().find("Cookie")->second
                                      : "")["session_id"];
    if (!this->isUserInSession(req, sessionIdFromBrowser)) {
      this->setStatusCode(FOUND);
      this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));
      this->setHeaders(req);
      return;
    }
  } else if (req.path == "/sign-in.html") {
    std::string sessionIdFromBrowser =
        this->parseFormURLEncoded(req.getRequest().count("Cookie")
                                      ? req.getRequest().find("Cookie")->second
                                      : "")["session_id"];
    if (this->isUserInSession(req, sessionIdFromBrowser)) {
      this->setStatusCode(FOUND);
      this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));
      this->setHeaders(req);
      return;
    }
  }

  // Generate response
  this->generateResponse(req);
}

// POST METHOD
void Response::POST_METHOD(Request &req) {
  // check rules if location is in config file
  if (!req.config.locations.empty()) {
    if (this->thisLocationIsInConfigFile(req, req.path)) {
      // redirection
      if (!req.config.locations[this->getIndexLocation()].return_to.empty()) {
        // Generate response (only headers)
        this->setIsRedirection(true);
        this->setStatusCode(FOUND);
        this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));
        this->setHeaders(req);

        return;
      }
      // check method
      if (this->checkAllowMethodsOfLocation(
              req.config.locations[this->getIndexLocation()].allow_methods,
              "post")) {
        // check if is file or directory
        bool isFile =
            this->isFile(req.config.locations[this->getIndexLocation()].root +
                         req.config.locations[this->getIndexLocation()].path);
        // check if the path is file
        if (!isFile) {
          this->setStatusCode(FORBIDDEN);
          req.path = req.config.error_page[FORBIDDEN];
          // Generate response
          this->generateResponse(req);
          return;
        }

      } else {
        this->setStatusCode(METHOD_NOT_ALLOWED);
        req.path = req.config.error_page[METHOD_NOT_ALLOWED];
        // Generate response
        this->generateResponse(req);
        return;
      }
    }
  }

  // get content type to decide which response will send in post method
  std::string contentType = req.getRequest().count("Content-Type")
                                ? req.getRequest().find("Content-Type")->second
                                : "";

  // application/x-www-form-urlencoded
  if (contentType == "application/x-www-form-urlencoded\r") {
    // set status code as default
    this->setStatusCode(OK);
    // set all data in html page
    this->addDataToBody(req);
    req.path = "post-request-data.html";
  } else if (contentType.substr(0, 52) == // multipart/form-data;
                                          // boundary=----WebKitFormBoundary
             "multipart/form-data; boundary=----WebKitFormBoundary") {
    std::string uploadBody = req.getRequest().count("binary-data")
                                 ? req.getRequest().find("binary-data")->second
                                 : "";
    // check if the file is empty
    if (uploadBody.empty()) {
      this->setStatusCode(FORBIDDEN);
      req.path = req.config.error_page[FORBIDDEN];
    } else if (uploadBody.length() / 1e6 >
               req.config
                   .client_max_body_size) { // check file size by config file
                                            // convert from bytes to MB
      this->setStatusCode(PAYLOAD_TOO_LARGE);
      req.path = req.config.error_page[PAYLOAD_TOO_LARGE];
    } else {
      std::string filename = req.getRequest().count("filename")
                                 ? req.getRequest().find("filename")->second
                                 : "";
      if (filename.empty()) {
        std::cerr << "Error: There no name on file!" << std::endl;
        return;
      }

      std::string fullPath = (req.config.root + "/uploads/");

      // check the directory of upload
      std::ifstream uploadDir(fullPath.c_str());
      if (!uploadDir.is_open()) {
        this->setStatusCode(FORBIDDEN);
        req.path = req.config.error_page[FORBIDDEN];
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

        // close output file
        uploadDir.close();
        outputFile.close();

        // status code
        this->setStatusCode(CREATED);

        // generate response page
        std::ofstream responseFile(
            (req.config.root + "/post-request-upload.html").c_str(),
            std::ios::out | std::ios::trunc);
        if (!responseFile.is_open()) {
          std::cerr << "file is not created(Upload file)" << std::endl;
          return;
        }

        std::string responseData =
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "  <head>\n"
            "    <meta charset=\"UTF-8\" />\n"
            "    <meta name=\"viewport\" content=\"width=device-width, "
            "initial-scale=1.0\" />\n"
            "    <script "
            "src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></"
            "script>\n"
            "    <title>POST Request Upload</title>\n"
            "  </head>\n"
            "\n"
            "  <body class=\"min-h-screen bg-black flex items-center "
            "justify-center\">\n"
            "    <!-- Card -->\n"
            "    <div\n"
            "      class=\"bg-zinc-900 border border-zinc-700 rounded-xl "
            "shadow-lg p-6 w-full max-w-xl\"\n"
            "    >\n"
            "\n"
            "      <h1 class=\"text-xl font-bold text-white text-center\">\n"
            "        File Uploaded Successfully ✅\n"
            "      </h1>\n"
            "    </div>\n"
            "  </body>\n"
            "</html>\n";

        responseFile << responseData;
        req.path = "post-request-upload.html";
      }
    }
  } else {
    this->setStatusCode(FORBIDDEN);
    req.path = req.config.error_page[FORBIDDEN];
  }

  // Generate response
  this->generateResponse(req);
}

// DELETE METHOD
void Response::DELETE_METHOD(Request &req) {

  // check rules if location is in config file
  if (!req.config.locations.empty()) {
    if (this->thisLocationIsInConfigFile(req, req.path)) {
      // redirection
      if (!req.config.locations[this->getIndexLocation()].return_to.empty()) {
        // Generate response (only headers)
        this->setIsRedirection(true);
        this->setStatusCode(FOUND);
        this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));
        this->setHeaders(req);

        return;
      }

      // check method
      if (!this->checkAllowMethodsOfLocation(
              req.config.locations[this->getIndexLocation()].allow_methods,
              "delete")) {
        this->setStatusCode(METHOD_NOT_ALLOWED);
        req.path = req.config.error_page[METHOD_NOT_ALLOWED];
        // Generate response
        this->generateResponse(req);
        return;
      }
    }
  }

  // check if we have a file or folder
  std::ifstream file((req.config.root + req.path).c_str());
  if (!file.is_open()) {
    this->setStatusCode(NOT_FOUND);
    req.path = req.config.error_page[NOT_FOUND];
  } else {
    if (std::remove((req.config.root + req.path).c_str()) == 0) {
      // set status code
      this->setStatusCode(NO_CONTENT);
      this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));
      this->setHeaders(req);
      return;
    }
    this->setStatusCode(FORBIDDEN);
    req.path = req.config.error_page[FORBIDDEN];
  }

  // Generate response
  this->generateResponse(req);
}

// Status code description
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
  } else if (statusCode == SOMTHING_WENT_WRONG) {
    return "503 Somthing Went Wrong";
  } else if (statusCode == GATEWAY_TIMEOUT) {
    return "504 Gateway Timeout";
  }
  return "Unknown Status";
}

// count the body length
size_t Response::countBodyLength(const std::string &path) {
  struct stat buffer;

  // get all status of path (size, type, ...)
  if (stat(path.c_str(), &buffer) == -1) {
    return 0;
  }

  return buffer.st_size;
}

// add data to body
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

  std::string afterPaseData =
      "    </div>\n"
      "  </div>\n"

      "  <!-- Redirect Button -->\n"
      "  <div class=\"mt-6 text-center\">\n"
      "    <button id=\"goBtn\"\n"
      "      class=\"bg-blue-600 hover:bg-blue-700 text-white font-semibold "
      "px-6 py-2 rounded transition\">\n"
      "      Go to Game (3)\n"
      "    </button>\n"
      "  </div>\n"

      "  <script>\n"
      "    let seconds = 3;\n"
      "    const btn = document.getElementById('goBtn');\n"
      "\n"
      "    const timer = setInterval(() => {\n"
      "      seconds--;\n"
      "      btn.textContent = `Go to Game (${seconds})`;\n"
      "\n"
      "      if (seconds <= 0) {\n"
      "        clearInterval(timer);\n"
      "        window.location.href = '/game.html';\n"
      "      }\n"
      "    }, 1000);\n"
      "\n"
      "    btn.addEventListener('click', () => {\n"
      "      window.location.href = '/game.html';\n"
      "    });\n"
      "  </script>\n"

      "</div>\n"
      "</body>\n"
      "</html>\n";

  // get the root path from config file
  std::string fullPath(req.config.root);

  fullPath.append("/post-request-data.html");

  // std::ios::out open the file for write event and if not exist created
  // std::ios::trunc remove all things in file
  std::ofstream file(fullPath.c_str(), std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "file is not open(addDataToBoddy)" << std::endl;
    return;
  }

  // insert all data in file
  file << beforRawData;

  // get post-body
  std::string body_post = req.getRequest().count("post-body")
                              ? req.getRequest().find("post-body")->second
                              : "";

  file << body_post;

  file << beforParseData;

  // parse body post
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

  // add raw data
  file << parseData;

  file << afterPaseData;
}

// parse form URL encoded
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

// check location is in config file
bool Response::thisLocationIsInConfigFile(Request &req, std::string &location) {
  // remove last char if he is '/'
  if (!location.empty() && location != "/" &&
      location[location.length() - 1] == '/') {
    location.erase(location.length() - 1);
  }

  for (size_t i = 0; i < req.config.locations.size(); i++) {
    if (req.config.locations[i].path == location) {
      this->setIndexLocation(i);
      return true;
    }
  }
  return false;
}

// check allow methods of location
bool Response::checkAllowMethodsOfLocation(
    std::vector<std::string> &allowMethods, std::string method) {
  // if the user don't specifec any methods so allow him to use all of them
  if (allowMethods.empty()) {
    return true;
  }

  // check if method is allowed;
  std::vector<std::string>::iterator it =
      std::find(allowMethods.begin(), allowMethods.end(), method);
  if (it != allowMethods.end()) {
    return true;
  }

  return false;
}

// generate page of
std::string Response::generatePageOfAutoIndex(Request &req,
                                              std::string &pathOfAutoIndex) {
  // open directory not file
  DIR *dir =
      opendir(pathOfAutoIndex
                  .c_str()); // DIR is data type of directory stream objects
  if (!dir) {                // if not open this directory than return null
    return req.config.error_page[NOT_FOUND];
  }

  // generate html of auto index
  std::string beforTitle =
      "<!DOCTYPE html>\n"
      "<html lang=\"en\">\n"
      "  <head>\n"
      "    <meta charset=\"UTF-8\" />\n"
      "    <meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1.0\" />\n"
      "    <script "
      "src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></script>\n"
      "    <title>Index of /uploads</title>\n"
      "  </head>\n"
      "  <body class=\"min-h-screen bg-slate-900 text-slate-100 p-10\">\n"
      "    <div class=\"max-w-3xl mx-auto\">\n";

  std::string title = "<h1 class=\"text-3xl font-bold mb-6\">\n"
                      "  📂 Index of <span class=\"text-blue-400\">" +
                      req.config.locations[this->getIndexLocation()].path +
                      "</span>\n"
                      "</h1>\n"

                      "<p class=\"text-sm text-slate-400 mb-6\">\n"
                      "  📁 Directory listing generated automatically\n"
                      "</p>\n"

                      "<div class=\"grid grid-cols-12 gap-4 border-b "
                      "border-slate-700 pb-2 mb-4 text-slate-400 text-sm\">\n"
                      "  <div class=\"col-span-6\">Name</div>\n"
                      "  <div class=\"col-span-3\">Type</div>\n"
                      "  <div class=\"col-span-3 text-right\">Size</div>\n"
                      "</div>\n"

                      "<ul class=\"space-y-2 text-sm\">\n";

  std::string footer =
      "<footer class=\"mt-10 text-center text-xs text-slate-500\">\n"
      "  ⚙️ webserv • autoindex\n"
      "</footer>\n"
      "</div>\n"
      "</body>\n"
      "</html>\n";
  std::string filesAndFolders;

  // this struct catch every file or folder in directory
  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (entry->d_name[0] == '.')
      continue;

    // Get file size in bytes
    size_t fileSize =
        this->countBodyLength(pathOfAutoIndex + "/" + entry->d_name);

    // Convert to KB for display
    size_t fileSizeKB = fileSize / 1024;
    if (fileSize % 1024 != 0)
      fileSizeKB += 1; // round up

    // Convert size to string
    std::stringstream ss;
    ss << fileSizeKB;
    std::string sizeOfEntry = ss.str();

    if (entry->d_type == DT_REG) {
      filesAndFolders.append(
          "<li>\n"
          "  <a\n"
          "    href=\"" +
          std::string(entry->d_name) +
          "\"\n"
          "    class=\"grid grid-cols-12 gap-4 hover:bg-slate-800 rounded px-2 "
          "py-1\"\n"
          "  >\n"
          "    <span class=\"col-span-6\">📄 " +
          std::string(entry->d_name) +
          "</span>\n"
          "    <span class=\"col-span-3 text-slate-400\">File</span>\n"
          "    <span class=\"col-span-3 text-right text-slate-400\">" +
          sizeOfEntry +
          " KB</span>\n"
          "  </a>\n"
          "</li>\n");
    } else if (entry->d_type == DT_DIR) {
      filesAndFolders.append(
          "<li>\n"
          "  <a\n"
          "    href=\"" +
          std::string(entry->d_name) +
          "/\"\n"
          "    class=\"grid grid-cols-12 gap-4 hover:bg-slate-800 rounded px-2 "
          "py-1\"\n"
          "  >\n"
          "    <span class=\"col-span-6 text-blue-400\">📁 " +
          std::string(entry->d_name) +
          "/</span>\n"
          "    <span class=\"col-span-3 text-slate-400\">Directory</span>\n"
          "    <span class=\"col-span-3 text-right text-slate-400\">—</span>\n"
          "  </a>\n"
          "</li>\n");
    }
  }

  // close directory to remove leaks
  closedir(dir);

  std::string htmlOfAutoIndex = beforTitle + title + filesAndFolders + footer;

  // generate the file to send fd to multiplx
  std::string fileName = "/autoindex.html";
  std::ofstream fileOfAutoIndex((req.config.root + fileName).c_str(),
                                std::ios::out | std::ios::trunc);
  if (!fileOfAutoIndex.is_open()) {
    return req.config.error_page[NOT_FOUND];
  }

  fileOfAutoIndex << htmlOfAutoIndex;

  return fileName;
}

// is start by slash
bool Response::isPathStartBySlash(const std::string &path) {
  if (!path.empty() && path[0] == '/') {
    return true;
  }
  return false;
}

// check if a file
bool Response::isFile(std::string path) {
  struct stat buffer;

  // get all status of path (size, type, ...)
  if (stat(path.c_str(), &buffer) == -1) {
    return false;
  }

  // check the path is file or not
  if (S_ISREG(buffer.st_mode)) {
    return true;
  }

  return false;
}

// is user in session
bool Response::isUserInSession(const Request &req, std::string session_id) {
  // get session id from server
  std::string sessionIdFromServer =
      req.getSession().count("session_id")
          ? req.getSession().find("session_id")->second
          : "";

  if ((sessionIdFromServer + "\r") == session_id) {
    return true;
  }

  return false;
}

// Method Not Allowed
void Response::methodNotAllowed(Request &req) {
  // set status code
  this->setStatusCode(BAD_REQUEST);

  // full path
  req.path = req.config.error_page[BAD_REQUEST];

  // Generate response
  this->generateResponse(req);
}

// Generate response
void Response::generateResponse(Request &req) {
  std::string fullPath;

  // root directory
  std::ifstream rootPath(req.config.root.c_str());
  if (!rootPath.is_open() ||
      // check root directory is out of folder of program
      (!req.config.root.empty() && req.config.root[0] == '.' &&
       req.config.root[1] == '.' && req.config.root[2] == '/')) {
    this->setStatusCode(NOT_FOUND);
    fullPath = "defaults/errors/404.html";
  } else {
    // handle slash after root path
    // we add slash only in path /
    if (!this->isPathStartBySlash(req.path)) {
      fullPath = req.config.root;
      fullPath.append("/");
    } else {
      fullPath = req.config.root;
    }

    // add path to root directory
    fullPath.append(req.path);
  }

  // check the file permissions and if the file exist
  if (access(fullPath.c_str(), F_OK) == -1 || !this->isFile(fullPath)) {
    this->setStatusCode(NOT_FOUND);
    fullPath = (req.config.root + "/" + req.config.error_page[NOT_FOUND]);
    // check if we have error page in root directory
    std::ifstream path(fullPath.c_str());
    if (!path.is_open()) {
      fullPath = "defaults/errors/404.html";
    }
  } else if (access(fullPath.c_str(), R_OK) == -1 ||
             access(fullPath.c_str(), W_OK) == -1) {
    this->setStatusCode(FORBIDDEN);
    fullPath = (req.config.root + "/" + req.config.error_page[FORBIDDEN]);
    // check if we have error page in root directory
    std::ifstream path(fullPath.c_str());
    if (!path.is_open()) {
      fullPath = "defaults/errors/403.html";
    }
  }

  // status line
  this->setStatusLine(req.httpV, statusCodeDescription(getStatusCode()));

  // Content Type
  this->setContentType(fullPath);

  // Content Length
  this->setContentLength(this->countBodyLength(fullPath));

  // merge all headers
  this->setHeaders(req);

  // get fd of body
  int fd = open(fullPath.c_str(), O_RDONLY);
  this->setBodyFd(fd);

  // close root path
  rootPath.close();
}

// Response
void Response::response(Request &req) {
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
