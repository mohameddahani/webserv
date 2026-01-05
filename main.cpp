/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 16:52:42 by mdahani           #+#    #+#             */
/*   Updated: 2026/01/05 17:58:29 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/webserv.hpp"

int main(int ac, char **av) {
  Request request;

  try {
    Server server;
    request.config.init_the_header_conf_default();
    if (ac > 2)
      throw std::runtime_error("./program [config file]");
    else if (ac == 2)
      request.config.parse_config_file(av[1]);
    server.run(request);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}