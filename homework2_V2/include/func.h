#ifndef _FUNC_H_
#define _FUNC_H_

#include <myhdr.h>
#include <my_structure.h>

// ----signal----
void sigchld_handler(int sig);
void set_signal_action();
// ----strProcessing----
void subServer(int sockfd, int c_fifofd, int s_fifofd, int id);
// ----structFunction----
void init_list(struct c_info (*list)[MAX_CLIENT]);
void addClient(struct c_info (*list)[MAX_CLIENT], struct c_info *newClient);
int changeC_Name(struct c_info (*list)[MAX_CLIENT], int id, char *newName);
int getMinC_ID(struct c_info list[MAX_CLIENT]);

// --processingStr--
int processingStr(char *str, char ****cmd, struct np **list, int sockfd);
// -----------------

// --ownCmdHandler--
int ownCmdHandler(char ****cmd, int sockfd);
// -----------------

// --parse--
int ParsePipe(char **str, char ***pipeStr);
void ParseSpace(char **str, char ***cmd);
bool isNum(char *str);
// ---------

// --exec--
void ExecArgs(char ***cmd, struct np **list);
void ExecArgsPipe(char ***cmd, struct np **list, int sockfd);
// --------

// --numPipeStruct--
void addNode(struct np ***list, char *str, int n);
void removeNode(struct np **list, struct np **target);
char ***numCmd(struct np **list, char ***cmd);
bool checkNumPipe(struct np **list);
void subNum(struct np **list);
void addNum(struct np **list);
// -----------------

#endif