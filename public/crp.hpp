//crp.hpp
#pragma once

#include "stdint.h"
#include <_types/_uint8_t.h>
#include <cstring>
#include <sys/socket.h>

namespace chatroom {
namespace net {
enum OP_CODE {
    CHAT,
    CAHT_GROUP,
    FILE_START,
    FILE_MID,
    FILE_END,
    LOGIN,
    LOGOUT
};

class CRPMessage{
private:
    // head
    uint16_t length;
    uint8_t op_code;
    uint32_t sender;
    uint32_t receiver;
    uint8_t data[4096 - 11]; //11 = length + op_code + sender + server

public:
    CRPMessage();
    CRPMessage(uint16_t length, uint8_t op_code, uint32_t sender,
             uint32_t receiver, char const *data);

    void DEBUG();
    int marshal(char*,int);//message写入网络 
    int unmarshal(char const *, int);//从网络还原message
    uint16_t get_length();
    uint32_t get_sender() const;
    uint32_t get_receiver() const;
    const uint8_t *get_data() const;
    uint8_t get_op_code() const;



    static int peek_length(char const *buf) { return ntohs(*(uint16_t *)buf);
}
};

class CRP{
private:
    int fd;
    char recv_buf[4096];
    char send_buf[4096];
    int recv_pointer;
    int send_pointer;
public:
    CRP(int);
    int receive(CRPMessage *);
    int send(CRPMessage *);
    int close();
    int get_fd();
    void set_fd(int);
    int get_send_pointer();

};

};
}
