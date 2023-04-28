#ifndef _MYHDR_H_
#define _MYHDR_H_

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <hiredis/hiredis.h>
#include <time.h>

#define SERV_PORT 6001
#define MAX_CLIENT 100
#define BUF_SIZE 5000
#define MSG 4000
#define NoOfCmd 5
#define MAX_CMD 256
#define SERVER_FIFO_TAMPLATE "/tmp/server.%d"
#define SERVER_FIFO_NAME_SIZE (sizeof(SERVER_FIFO_TAMPLATE) + 20)
#define FIFO_TAMPLATE "/tmp/client.%d"
#define FIFO_NAME_SIZE (sizeof(FIFO_TAMPLATE) + 20)

extern time_t t;

#endif