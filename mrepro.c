#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <err.h>
#include "mrepro.h"
#include <netdb.h>
#include <arpa/inet.h>

#define ERR_NO 1

int Socket(int domain, int type, int protocol) {
  int sock_fd = socket(domain, type, protocol);
  if(sock_fd < 0) {
    errx(ERR_NO, "Socket initialization failed.");
  }

  return sock_fd;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
  int sent;
  sent = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
  if(sent < 0) {
    errx(ERR_NO, "Error while sending datagram.");
  }

  return sent;
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
  ssize_t msg_len;
  msg_len = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
  if(msg_len < 0) {
    errx(ERR_NO, "Error while receiving datagram.");
  }

  return msg_len;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int result = bind(sockfd, addr, addrlen);
  if(result == -1) {
    errx(ERR_NO, "Binding error.");
  }

  return result;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int result = connect(sockfd, addr, addrlen);
  if(result == -1) {
    errx(ERR_NO, "Connect error.");
  }

  return result;
}

int Listen(int sockfd, int backlog) {
  int result = listen(sockfd, backlog);
  if(result == -1) {
    errx(ERR_NO, "Listen error.");
  }

  return result;
}

int Accept(int socket, struct sockaddr *cliaddr, socklen_t *addrlen) {
  int newfd = accept(socket, cliaddr, addrlen);
  if(newfd == -1) {
    errx(1, "Accept error.");
  }

  return newfd;
}

int Getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res) {
  int result = getaddrinfo(node, service, hints, res);
  if(result < 0) {
    errx(ERR_NO, "%s", gai_strerror(result));
  }

  return result;
}

int Inet_pton(int af, const char *src, void *dst) {
  int result = inet_pton(af, src, dst);
  if(result != 1) {
    errx(ERR_NO, "%s is not a valid IP address.", src);
  }

  return result;
}

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size) {
  const char *result = inet_ntop(af, src, dst, size);
  if(!result) {
    errx(ERR_NO, "Invalid representation of IP address.");
  }

  return result;
}

ssize_t readn(int fd, void *vptr, size_t n) {
  size_t nleft;
  ssize_t nread;
  char *ptr;

  ptr = vptr;
  nleft = n;

  while(nleft > 0) {
    if((nread = read(fd, ptr, nleft)) < 0) {
      if(errno == EINTR) {
        nread = 0;
      } else {
        errx(ERR_NO, "Error while sending data.");
      }
    } else if(nread == 0) {
      break;
    }
    nleft -= nread;
    ptr += nread;
  }

  return (n - nleft);
}

ssize_t writen(int fd, const void *vptr, size_t n) {
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = vptr;
  nleft = n;

  while(nleft > 0) {
    if((nwritten = write(fd, ptr, nleft)) <= 0) {
      if(nwritten < 0 && errno == EINTR) {
        nwritten = 0;
      } else {
        errx(ERR_NO, "Error while sending data.");
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }

  return n;
}
