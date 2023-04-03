#ifndef _MY_STRUCTURE_H_
#define _MY_STRUCTURE_H_

#include <myhdr.h>

struct c_info{
    int user_id;
    // Change 2 使用固定空間避免記憶體空間洩漏等問題
    // 故後續動到user_name的都屬於Change 2
    char user_name[64];
    // char *user_name;
    char *user_ip;
    uint16_t user_port; 
};

struct np
{
    int num;
    char *cmd;
    struct np *next;
};

#endif