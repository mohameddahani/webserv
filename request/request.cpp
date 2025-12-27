/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/24 20:48:07 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/27 13:15:29 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// * Setters & Getters
void Request::setRequest(const std::string &req) {
  std::stringstream ss(req);

  // * get method & path & http version
  std::string line;
  if (std::getline(ss, line)) {
    std::stringstream firstLine(line);
    std::string method;
    firstLine >> method;
    if (method == "GET") {
      this->method = GET;
    } else if (method == "POST") {
      this->method = POST;
    } else if (method == "DELETE") {
      this->method = DELETE;
    }

    firstLine >> this->path >> this->httpV;
  }

  // * store request on map
  std::string key, value;
  while (std::getline(ss, line)) {
    size_t pos = line.find(":");
    if (pos == std::string::npos) {
      // * this case for store post header
      if (this->method == POST) {
        std::getline(ss, line);
      } else {
        continue;
      }
    }

    // * create a key and value by method
    if (this->method == POST) {
      key = "post-body";
      value = line;
    } else {
      key = line.substr(0, pos);
      value = line.substr(pos + 2, line.length());
    }

    this->request[key] = value;
  }
}

const std::map<std::string, std::string> &Request::getRequest() const {
  return this->request;
}