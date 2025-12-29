/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/24 20:48:07 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/29 17:23:29 by mdahani          ###   ########.fr       */
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
    std::string reqMethod;
    firstLine >> reqMethod;
    if (reqMethod == "GET") {
      this->method = GET;
    } else if (reqMethod == "POST") {
      this->method = POST;
    } else if (reqMethod == "DELETE") {
      this->method = DELETE;
    } else {
      this->method = ELSE;
    }

    firstLine >> this->path >> this->httpV;
  }

  // * get the headers
  std::string key, value;
  while (std::getline(ss, line)) {
    size_t pos = line.find(":");
    if (pos == std::string::npos) {
      break;
    }

    key = line.substr(0, pos);
    value = line.substr(pos + 2, line.length());

    this->request[key] = value;
  }

  // * get the body
  key = "post-body";
  value.clear();
  while (std::getline(ss, line)) {
    value += line;
  }

  if (!value.empty()) {
    this->request[key] = value;
  }
}

const std::map<std::string, std::string> &Request::getRequest() const {
  return this->request;
}