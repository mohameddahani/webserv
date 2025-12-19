/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 15:59:16 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/19 10:44:50 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// * includes
#include <iostream>

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
  Webserv &operator=(const Webserv &other) {}

  // * Destructor
  ~Webserv() {}
};

// ! RESPONSE
class Response {
  // ! private
private:
  // ! public
public:
  // * Default Constructor
  Response() {}

  // * Copy Constructor
  Response(const Response &other) {}

  // * Copy assignment operator
  Response &operator=(const Response &other) {}

  // * Destructor
  ~Response() {};
};

#endif