/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 13:05:03 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/19 21:39:50 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

#define PORT 3000
#define IP INADDR_ANY

int main() {
  // TODO: SERVER SIDE
  // ! Create a Socket
  // * A socket is one endpoint of a two way communication link between two
  // * programs running on the network.
  // ? ARGS:
  // * 1st => is the domain name like he choose the type of ip you will use
  // * like (ipv4 or ipv6).
  // * 2nd => type like determines how data is transferred.
  // * 3rd => is protocol like This specifies the protocol within the domain
  // * type. like (0 the system will chose it auto)

  // * here we choose:
  // * AF_INET (ipv4)
  // * SOCK_STREAM (TCP)
  // * 0 (auto)
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "server is not running !" << std::endl;
    return 1;
  }

  // ! Create a buffer to store ip address and port
  // * let add an address ip and port to server
  struct sockaddr_in sockaddr;
  sockaddr.sin_addr.s_addr = htonl(IP);
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(PORT); // * Host TO Network Short we use it to
                                   // * convert number (16bit) to network format
  memset(sockaddr.sin_zero, 0, sizeof(sockaddr_in));
  // * This field is just padding to make the struct size match the generic
  // * sockaddr.
  // * We zero it to avoid any garbage values that could cause unexpected
  // * behavior in bind().

  // * len of sockaddr
  int addrLen = sizeof(sockaddr);

  // ! Link the address ip and PORT with socket
  // * The bind() method is associated with a socket, with a specific
  // * local address and port number which allows the socket to listen for
  // * incoming connection on that address.
  if (bind(sockfd, (struct sockaddr *)&sockaddr, addrLen) < 0) {
    std::cerr << "bind failed" << std::endl;
    return 1;
  }

  // ! Listen
  // * The listen() function marks the socket as a passive socket which prepares
  // * a socket to accept incoming connection requests (for servers).
  // ? the second param is listen is how many request can get in same time
  // ? Maximum number of pending connections in the queue before rejecting a new
  // ? one
  if (listen(sockfd, 10) < 0) {
    std::cerr << "listen failed" << std::endl;
    return 1;
  }

  // ! accepts a new connection from a client (for servers).
  // * The accept() function accepts a new connection from a client (for
  // * servers). It extracts the first connection request on the queue of
  // * pending connections and creates a new socket for that connection.
  while (true) {
    int newSockFd =
        accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t *)&addrLen);
    if (newSockFd < 0) {
      std::cerr << "accept failed" << std::endl;
      return 1;
    }
    // ! Recive request from browser
    // * buffer that we store to him the request
    char buffer[4096];
    // * The recv() function is a system call that is used to receive data from
    // * a connected socket which allows the client or server to read incoming
    // * messages.
    // * and is return the number of bytes that read
    int bytesRead = recv(newSockFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead > 0) {
      buffer[bytesRead] = '\0';
      std::cout << buffer << std::endl;
      response(newSockFd);
      close(newSockFd);
    }
  }
}