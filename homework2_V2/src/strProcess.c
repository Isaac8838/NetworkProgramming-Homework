#include <myhdr.h>
#include <func.h>
#include <my_structure.h>

void subServer(int sockfd, int c_fifofd, int s_fifofd, int id) {
    char *buf = malloc(sizeof(char *) * BUF_SIZE);
    char **cmd = (char **)malloc(sizeof(char *) * BUF_SIZE);
    fd_set all_fds;
    int max_fd, num_byte;
    
    // Change 3 最重要的部分 （主要的BUG
    // 每次都必須把fd_set 歸零


    while (1) {
        FD_ZERO(&all_fds);
        FD_SET(sockfd, &all_fds);
        FD_SET(s_fifofd, &all_fds);
        if (sockfd > s_fifofd) max_fd = sockfd;
        else max_fd = s_fifofd;
        write(sockfd, "% ", strlen("% "));

        if (select(max_fd + 1, &all_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        memset(buf, 0, sizeof(char *) * BUF_SIZE);

        if (FD_ISSET(sockfd, &all_fds)) {
            if ((num_byte = read(sockfd, buf, BUF_SIZE)) < 0) {
                if (errno == ECONNRESET) {
                    perror("sockfd");
                    continue;
                }
            }
            char *temp = strdup(buf);
            temp[num_byte - 2] = '\0';
            cmd[0] = strsep(&temp, " ");
            if (strcmp(cmd[0], "who") == 0) {
                struct c_info users[MAX_CLIENT];
                char top_info[BUF_SIZE];
                write(c_fifofd, cmd[0], strlen(cmd[0]));
                read(s_fifofd, users, sizeof(struct c_info) * MAX_CLIENT);
                snprintf(top_info, BUF_SIZE, "<ID>    <name>     <IP:port>          <indicate me>\n");
                write(sockfd, top_info, strlen(top_info));
                for (int i = 0; i < MAX_CLIENT; i++) {
                    if (users[i].user_id == -1) continue;
                    if (users[i].user_id == id)
                        snprintf(top_info, BUF_SIZE, "   %d    %s    %s:%d    <-(me)\n", users[i].user_id, users[i].user_name, users[i].user_ip, users[i].user_port);
                    else
                        snprintf(top_info, BUF_SIZE, "   %d    %s    %s:%d\n", users[i].user_id, users[i].user_name, users[i].user_ip, users[i].user_port);
                    write(sockfd, top_info, strlen(top_info));
                }
            }
            else if (strcmp(cmd[0], "tell") == 0) {
                int ack = 0;
                char message[64] = {};
                cmd[1] = strsep(&temp, " ");
                write(c_fifofd, cmd[0], strlen(cmd[0]));
                read(s_fifofd, &ack, sizeof(int));
                if (ack)
                    write(c_fifofd, cmd[1], strlen(cmd[1]));
                else
                    write(sockfd, "tell error", strlen("tell error"));

                read(s_fifofd, &ack, sizeof(int));

                if (ack)
                    write(c_fifofd, temp, strlen(temp));
                else
                    write(sockfd, "tell error", strlen("tell error"));
                read(s_fifofd, message, 64);
                write(sockfd, message, strlen(message));
            }
            else if (strcmp(cmd[0], "yell") == 0) {
                int ack = 0;
                char msg[BUF_SIZE];
                write(c_fifofd, cmd[0], strlen(cmd[0]));
                read(s_fifofd, &ack, sizeof(int));
                if (ack)
                    write(c_fifofd, temp, strlen(temp));
                else
                    write(sockfd, "yell error", strlen("yell error"));
                // read(s_fifofd, msg, BUF_SIZE);
                // write(sockfd, msg, strlen(msg));
            }
            else if (strcmp(cmd[0], "name") == 0) {
                int ack = 0;
                // change 4 clear msg , 避免buffer殘留
                char msg[BUF_SIZE] = {0};
                
                write(c_fifofd, cmd[0], strlen(cmd[0]));
                read(s_fifofd, &ack, sizeof(int));
                if (ack)
                    write(c_fifofd, temp, strlen(temp));
                else 
                    write(sockfd, "name error", strlen("name error"));
                read(s_fifofd, msg, BUF_SIZE);
                write(sockfd, msg, strlen(msg));
            }
            else if (strcmp(cmd[0], "quit") == 0) {
                write(c_fifofd, cmd[0], strlen(cmd[0]));
                close(sockfd);
                close(s_fifofd);
                close(c_fifofd);
                exit(EXIT_SUCCESS);
            }
            else {
                for (int i = 0; i < strlen(buf); i++) {
                    if (buf[i] == '\r') {buf[i] = '\0'; break;}
                }
                int typeOfExec = 0;
                setenv("PATH", "bin:.:/bin", 1);
                struct np *list = NULL;
                char ***cmd;
                cmd = (char ***)malloc(MAX_CMD * sizeof(char **));
                for (int i = 0; i < MAX_CMD; i++) {
                    cmd[i] = NULL;
                }
                subNum(&list);
                typeOfExec = processingStr(buf, &cmd, &list, sockfd);
                switch(typeOfExec) {
                    case 1:
                        ExecArgsPipe(cmd, &list, sockfd);
                        break;
                    case 2:
                        struct np **temp = &list;
                        while (*temp != NULL) {
                            if ((*temp)->num == 0) {
                                ExecArgsPipe(numCmd(&(*temp), cmd), &list, sockfd);
                                if ((*temp)->num == 0) removeNode(&list, &(*temp));
                                temp = &list;
                            }
                            else temp = &((*temp)->next);
                        }
                    default:
                        break;
                }
            }
        }
        if (FD_ISSET(s_fifofd, &all_fds)) {
            read(s_fifofd, buf, BUF_SIZE);
            FD_CLR(sockfd, &all_fds);
            write(sockfd, buf, strlen(buf));
            FD_SET(sockfd, &all_fds);
        }
    }
    free(buf);
}