/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 16:52:42 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/25 09:26:44 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/webserv.hpp"

int main(int ac, char **av) {
  Request request;

  try {
    Server server;
    request.init_the_header_conf_default(request);
    if (ac > 2)
      throw std::runtime_error("./program [config file]");
    else if (ac == 2)
      request.parse_config_file(request, av[1]);
    server.run(request);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}