#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include "mrepro.h"

#define DEFAULT_PORT "1234"
#define MAX_FILENAME_LEN 256
#define FILE_BUF_LEN 1024
#define BACKLOG 10

char *read_filename(int conn_fd) {
  char *filename, *result;
  int n_read = 0, total_read = 0;

  filename = (char *)malloc(MAX_FILENAME_LEN * sizeof(char));
  result = filename;
  while(1) {
    if((n_read = read(conn_fd, filename, MAX_FILENAME_LEN)) < 0) {
      if(errno == EINTR) {
        n_read = 0;
      } else {
        errx(1, "Filename reading error.");
      }
    } else if(n_read == 0) {
      errx(1, "Connection closed before request was processed.");
    }

    filename += n_read;
    total_read += n_read;

    if(*(filename - 1) == '\0') {
      break;
    }
  }

  return result;
}

void write_result(int conn_fd, uint8_t code, void *data, size_t size) {
  writen(conn_fd, &code, 2);
  writen(conn_fd, data, size);
}

void write_file(int conn_fd, FILE *file, int file_offset) {
  char *file_data;
  uint8_t res_status = 0;
  size_t read_bytes = 0;

  file_data = (char *)malloc(FILE_BUF_LEN * sizeof(char));
  writen(conn_fd, &res_status, 2);
  fseek(file, file_offset, 0L);

  while((read_bytes = fread(file_data, sizeof(char), FILE_BUF_LEN, file)) > 0) {
      writen(conn_fd, file_data, read_bytes);
  }
}

void process_requests(int sock_fd) {
  int conn_fd, file_offset;
  struct sockaddr_in cl_addr;
  socklen_t cl_size;
  char *filename, *error_message;
  FILE *file;

  while(1) {
    cl_size = sizeof(cl_addr);
    conn_fd = Accept(sock_fd, (struct sockaddr *)&cl_addr, &cl_size);

    if(readn(conn_fd, &file_offset, 4) == -1) {
      errx(1, "Offset reading error.");
    }
    file_offset = ntohl(file_offset);
    filename = read_filename(conn_fd);

    if(strchr(filename, '/') || access(filename, F_OK)) {
      error_message = "File does not exist.\n";
      write_result(conn_fd, 0x01, error_message, strlen(error_message));
      close(conn_fd);
      continue;
    }
    if(access(filename, R_OK)){
      error_message = "No read permission for file.\n";
      write_result(conn_fd, 0x02, error_message, strlen(error_message));
      close(conn_fd);
      continue;
    }

    file = fopen(filename, "r");
    write_file(conn_fd, file, file_offset);
    fclose(file);

    close(conn_fd);
  }
}

int main(int argc, char *argv[]) {
  int opt, sock_fd;
  char *port = DEFAULT_PORT;
  struct addrinfo hints, *res;

  while((opt = getopt(argc, argv, "p:")) != -1) {
    switch(opt) {
      case 'p':
        port = optarg;
        break;
      default:
        exit(1);
    }
  }

  if(optind != argc) {
    errx(1, "Usage: %s [-p port]", argv[0]);
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_flags |= AI_PASSIVE;
  Getaddrinfo(NULL, port, &hints, &res);

  sock_fd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  Bind(sock_fd, res->ai_addr, res->ai_addrlen);
  Listen(sock_fd, BACKLOG);
  process_requests(sock_fd);
  close(sock_fd);

  return 0;
}
