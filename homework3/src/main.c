#include <myhdr.h>
#include <my_structure.h>
#include <func.h>
#include <my_redis.h>

time_t t;

int main() {
    int listenfd, connectfd, c_fifofd[MAX_CLIENT], s_fifofd[MAX_CLIENT];
    int min_c_id;
    __pid_t pid;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
    fd_set read_fds, all_fds;
    struct c_info list[MAX_CLIENT];
    char client_fifo[FIFO_NAME_SIZE], server_fifo[SERVER_FIFO_NAME_SIZE], buf[BUF_SIZE];
    t = time(NULL);
    
    set_signal_action();
    init_list(&(list));
    initRedis();
    

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // Change 1
    // 加這行避免結束server後bind : Address in use , 使addr可以被重用
    int opt = 1;
    if( setsockopt((listenfd), SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_CLIENT; i++) {
        s_fifofd[i] = -1;
        c_fifofd[i] = -1;
    }

    FD_ZERO(&all_fds);
    FD_SET(listenfd, &all_fds);
    int max_fd = listenfd;

    while (1) {
        clilen = sizeof(cliaddr); 
        
        read_fds = all_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) continue;
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(listenfd, &read_fds)) {
            connectfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
            min_c_id = getMinC_ID(list);
            struct c_info *newClient = malloc(sizeof(struct c_info));
            newClient->user_id = min_c_id;
            // newClient->user_name = strdup("no_name");
            strcpy(newClient->user_name , "no_name");
            newClient->user_ip = inet_ntoa(cliaddr.sin_addr);
            newClient->user_port = ntohs(cliaddr.sin_port);
            char *username = login(connectfd);
            strcpy(newClient->user_name, username);
            addClient(&(list), newClient);
  

            // make two fifos for each child server
            snprintf(server_fifo, SERVER_FIFO_NAME_SIZE, SERVER_FIFO_TAMPLATE, min_c_id);
            snprintf(client_fifo, FIFO_NAME_SIZE, FIFO_TAMPLATE, min_c_id);
            umask(0);
            if (mkfifo(server_fifo, 0666) == -1 && errno != EEXIST) {
                perror("sever fifo error");
                exit(EXIT_FAILURE);
            }
            if (mkfifo(client_fifo, 0666) == -1 && errno != EEXIST) {
                perror("client fifo error");
                exit(EXIT_FAILURE);
            }

            // create a child process server for each client
            if ((pid = fork()) == 0) {
                close(listenfd);
                if ((s_fifofd[min_c_id] = open(server_fifo, O_RDONLY)) < 0) {
                    fprintf(stderr, "open %s error\n", server_fifo);
                    exit(EXIT_FAILURE);
                }

                if ((c_fifofd[min_c_id] = open(client_fifo, O_WRONLY)) < 0) {
                    fprintf(stderr, "open %s error\n", client_fifo);
                    exit(EXIT_FAILURE);
                }

                for (int i = 0; i < MAX_CLIENT; i++) {
                    if (i == min_c_id || s_fifofd[i] == -1 || c_fifofd[i] == -1) continue;
                    close(s_fifofd[i]);
                    close(c_fifofd[i]);
                    s_fifofd[i] = -1;
                    c_fifofd[i] = -1;
                }

                subServer(connectfd, c_fifofd[min_c_id], s_fifofd[min_c_id], min_c_id, username);
                exit(EXIT_SUCCESS);
            }

            // open two fifos for each child server
            if ((s_fifofd[min_c_id] = open(server_fifo, O_WRONLY)) < 0) {
                fprintf(stderr, "open %s error: %s\n", server_fifo, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if ((c_fifofd[min_c_id] = open(client_fifo, O_RDONLY)) < 0) {
                fprintf(stderr, "open %s error: %s\n", client_fifo, strerror(errno));
                exit(EXIT_FAILURE);
            }

            FD_SET(c_fifofd[min_c_id], &all_fds);
            if (c_fifofd[min_c_id] > max_fd) max_fd = c_fifofd[min_c_id];
            
            close(connectfd);

        }
        
        // check each client fifo if it need handle data
        for (int i = 0; i < MAX_CLIENT; i++) {
            if (FD_ISSET(c_fifofd[i], &read_fds)) {
                memset(buf, 0, sizeof(buf));

                int num_byte = read(c_fifofd[i], buf, BUF_SIZE);

                if (num_byte == -1) {
                    perror("num_byte");
                    exit(EXIT_FAILURE);
                }
                else if (num_byte == 0) {
                    close(c_fifofd[i]);
                    FD_CLR(c_fifofd[i], &all_fds);
                    c_fifofd[i] = -1;
                    list[i].user_id = -1;
                }
                else {
                    // buf[num_byte - 2] = '\0';
                    if (strcmp(buf, "who") == 0) {
                        write(s_fifofd[i], list, sizeof(struct c_info) * MAX_CLIENT);
                    }
                    else if (strcmp(buf, "tell") == 0) {
                        char id[100] = {};
                        char message[MSG] = {};
                        char tellmsg[BUF_SIZE] = {};
                        int ack = 1;
                        write(s_fifofd[i], &ack, sizeof(int));
                        read(c_fifofd[i], id, 100);
                        write(s_fifofd[i], &ack, sizeof(int));
                        read(c_fifofd[i], message, MSG);
                        write(s_fifofd[i], "send accept!\n", strlen("send accept!\n"));
                        snprintf(tellmsg, BUF_SIZE, "<user(%d) told you>: %s\n", i, message);
                        int c_id = atoi(id);
                        write(s_fifofd[c_id], tellmsg, strlen(tellmsg));
                    }
                    else if (strcmp(buf, "yell") == 0) {
                        char message[MSG] = {};
                        char yellmsg[BUF_SIZE] = {};
                        int ack = 1;
                        write(s_fifofd[i], &ack, sizeof(int));
                        read(c_fifofd[i], message, MSG);
                        snprintf(yellmsg, BUF_SIZE, "<user(%d) yelled>: %s\n", i, message); 
                        for (int j = 0; j < MAX_CLIENT; j++) {
                            if (!(s_fifofd[j] == -1)) {
                                write(s_fifofd[j], yellmsg, strlen(yellmsg));
                            }
                        }
                    }
                    else if (strcmp(buf, "name") == 0) {
                        char name[100] = {};
                        char msg[BUF_SIZE] = {};
                        int ack = 1;
                        write(s_fifofd[i], &ack, sizeof(int));
                        read(c_fifofd[i], name, 100);
                        if (changeC_Name(&(list), i, name)) {
                            snprintf(msg, BUF_SIZE, "name change accept!\n");
                            write(s_fifofd[i], msg, strlen(msg));
                        }
                        else {
                            snprintf(msg, BUF_SIZE, "User %s already exists!\n", name);
                            write(s_fifofd[i], msg, strlen(msg));
                        }
                    }
                    else if (strcmp(buf, "gyell") == 0) {
                        char msg[4096] = {};
                        char groupName[256] = {};
                        int ack = 1, len = 0;
                        write(s_fifofd[i], &ack, sizeof(int));
                        read(c_fifofd[i], msg, sizeof(char) * 4096);
                        write(s_fifofd[i], &ack, sizeof(int));
                        read(c_fifofd[i], groupName, sizeof(char) * 256);

                        redisReply *tr = redisCommand(rc, "ZRANGE %s 0 -1", groupName);
                        char online[MAX_CLIENT][64] = {};
                        if (tr && tr->type == REDIS_REPLY_ARRAY) {
                            redisReply **r = tr->element;
                            len = tr->elements;
                            for (int j = 0; j < len; j++) {
                                strcpy(online[j], r[j]->str);
                            }
                        }
                        for (int j = 0; j < len; j++) {
                            for (int k = 0; k < MAX_CLIENT; k++) {
                                if (s_fifofd[k] != -1 && strcmp(list[k].user_name, online[j]) == 0) {
                                    write(s_fifofd[k], msg, strlen(msg));
                                    break;
                                }
                            }
                        }
                        freeReplyObject(tr);
                    }
                    else if (strcmp(buf, "quit") == 0) {
                        FD_CLR(c_fifofd[i], &all_fds);
                        close(c_fifofd[i]);
                        close(s_fifofd[i]);
                        c_fifofd[i] = -1;
                        s_fifofd[i] = -1;
                        list[i].user_id = -1;
                    }
                }
            }
        }
    }
    return 0;
}