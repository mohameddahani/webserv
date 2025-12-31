/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 13:13:15 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/31 13:24:24 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/webserv.hpp"

void Webserv::throwError(std::string func) {
  throw std::runtime_error(func + "failed: " + strerror(errno));
}