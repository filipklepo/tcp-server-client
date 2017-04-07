#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include "mrepro.h"

#define DEFAULT_SERV_ADDR "127.0.0.1"
#define DEFAULT_PORT "1234"
#define RECV_BUF_LEN 1024

void communicate(int sock_fd, char *filename, int c_flag) {
  uint32_t offset = 0;
  uint8_t res_status;
  struct stat st;
  FILE *file;
  int read_num;
  char recv_buf[RECV_BUF_LEN];

  if(c_flag && !access(filename, F_OK)) {
    stat(filename, &st);
    offset = st.st_size;
  }
  offset = htonl(offset);
  writen(sock_fd, &offset, 4);
  writen(sock_fd, filename, strlen(filename) + 1);

  readn(sock_fd, &res_status, 1);
  if(res_status != 0) {
    while(1) {
      if((read_num = read(sock_fd, recv_buf, RECV_BUF_LEN)) < 0) {
        if(errno != EINTR) {
          errx(1, "Error while receiving error message.");
        }
      } else if(read_num == 0) {
        break;
      }

      fwrite(recv_buf, sizeof(char), read_num, stdout);
    }

    exit(res_status);
  }

  file = fopen(filename, "a");
  while(1) {
    if((read_num = read(sock_fd, recv_buf, RECV_BUF_LEN)) < 0) {
      if(errno != EINTR) {
        errx(1, "Error while receiving file content.");
      }
    } else if(read_num == 0) {
      break;
    }

    fwrite(recv_buf, sizeof(char), read_num, file);
  }
  fclose(file);
}

int main(int argc, char *argv[]) {
  int opt, sock_fd, c_flag = 0;
  char *serv_addr = DEFAULT_SERV_ADDR, *port = DEFAULT_PORT, *filename;
  struct addrinfo hints, *res;

  while((opt = getopt(argc, argv, "s:p:c")) != -1) {
    switch(opt) {
      case 's':
        serv_addr = optarg;
        break;
      case 'p':
        port = optarg;
        break;
      case 'c':
        c_flag = 1;
        break;
      default:
        exit(1);
    }
  }

  if(optind != argc - 1) {
    errx(1, "Usage: %s [-s server] [-p port] [-c] filename", argv[0]);
  }
  filename = argv[optind];

  if(!c_flag && !access(filename, R_OK)) {
    errx(1, "File %s already exists.", filename);
  }
  if(c_flag && access(filename, W_OK) && !access(filename, F_OK)) {
    errx(1, "Missing permission to write to file %s.", filename);
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_flags |= AI_PASSIVE;
  Getaddrinfo(serv_addr, port, &hints, &res);

  sock_fd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  Connect(sock_fd, res->ai_addr, res->ai_addrlen);
  communicate(sock_fd, filename, c_flag);
  close(sock_fd);

  return 0;
}
