/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 13:05:03 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/26 10:11:05 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

// * Methods

void Server::run() {
  // * Request
  Request req;

  // * Response
  Response res;

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
    throw std::runtime_error("failed to create a socket !");
  }
  int option = 1;

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  // ! Define a ip and port for socket
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
    throw std::runtime_error("bind failed");
  }

  // ! Listen
  // * The listen() function marks the socket as a passive socket which prepares
  // * a socket to accept incoming connection requests (for servers).
  // ? the second param is listen is how many request can get in same time
  // ? Maximum number of pending connections in the queue before rejecting a new
  // ? one
  if (listen(sockfd, MAX_LISTEN) < 0) {
    throw std::runtime_error("listen failed");
  }

  // ! Monitor the client that connect to the server
  // ? The epoll API monitoring multiple
  // ? file descriptors to see if I/O is possible on any of them.

  // * this creates a new epoll instance and returns a file descriptor referring
  // * to that instance.
  // ? EPOLL_CLOEXEC this mean The kernel automatically closes epfd during
  // ? execve() ? If this process calls `execve()`, automatically close `epfd`.
  int epfd = epoll_create1(EPOLL_CLOEXEC);
  if (epfd < 0) {
    std::cerr << "epoll failed" << std::endl;
  }

  // * epoll_event It is a structure defined by the Linux system, used to
  // * describe an event that you want epoll to monitor or report to you.
  struct epoll_event ev;
  // * Event type that we need to watch (read, write, error...)
  ev.events = EPOLLIN; // * is ready for EPOLLIN (Read) || EPOLLOUT (Write)
  // * Event data (usually FD)
  ev.data.fd = sockfd; // * that will epoll_ctl return if is ready for
                       // * (Read or Write)

  // * epoll_ctl is list of fds that we need to watch it on epoll by events
  // * rules
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
    std::cerr << "epoll_ctl failed" << std::endl;
  }

  // * this array of struct used by epoll_wait() to store ready file descriptor
  // * events for EPOLLIN (Read) || EPOLLOUT (Write)
  struct epoll_event events[MAX_EVENTS];

  // ! accepts a new connection from a client (for servers).

  // ? buffer that we store to him the request
  char buffer[MAX_BUFFER_SIZE];
  while (true) {

    // * epoll_wait fill the array events by ready fds and return the fds that's
    // * ready for (Read) || (Write)
    // ? -1 is arg for time out and when we use -1 that's mean we still
    // ? wait untill get an fd
    int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

    if (n < 0) {
      std::cerr << "epoll_wait failed" << std::endl;
    }

    // ! Recive request from browser
    for (int i = 0; i < n; i++) {
      // * check if we have fd of server
      if (events[i].data.fd == sockfd) {
        // * The accept() function accepts a new connection from a client (for
        // * servers). It extracts the first connection request on the queue of
        // * pending connections and creates a new socket for that connection.
        int clientFd =
            accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t *)&addrLen);
        if (clientFd < 0) {
          std::cerr << "accept failed" << std::endl;
        }

        // * add new sockfd of client to watch mode
        struct epoll_event clientEv;
        clientEv.events = EPOLLIN;
        clientEv.data.fd = clientFd;

        epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &clientEv);
      } else {
        if (events[i].events & EPOLLIN) { // * EPOLLIN o r EPOLLERR
          int clientFd = events[i].data.fd;

          // * The recv() function is a system call that is used to receive data
          // * from a connected socket which allows the client or server to read
          // * incoming messages. and is return the number of bytes that read
          // ? 0 mean read data normaly ()
          int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
          buffer[bytesRead] = '\0';
          if (bytesRead <= 0) {
            epoll_ctl(epfd, EPOLL_CTL_DEL, clientFd, NULL);
            close(clientFd);
          }
          std::cout << "==========Request==========" << std::endl;
          std::string request = buffer;
          std::cout << request << std::endl;
          req.setRequest(request);
          // * add new sockfd of client to watch mode again
          struct epoll_event clientEv;
          clientEv.events = EPOLLIN | EPOLLOUT;
          clientEv.data.fd = clientFd;

          epoll_ctl(epfd, EPOLL_CTL_MOD, clientFd, &clientEv);
        }
        if (events[i].events & EPOLLOUT) { // * EPOLLIN or EPOLLERR
          std::string request = buffer;

          // * Response
          res.response(events[i].data.fd, req);

          epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          close(events[i].data.fd);
        }
        if (events[i].data.fd & EPOLLERR) {
          epoll_ctl(events[i].data.fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          close(events[i].data.fd);
        }
      }
    }
  }
}
