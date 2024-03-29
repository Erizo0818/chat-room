// crp.cpp
#include "crp.hpp"
#include <_types/_uint8_t.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace chatroom {
namespace net {
//---------------------CRPMessage-------------------

CRPMessage::CRPMessage() { memset(this, 0, sizeof(CRPMessage)); }

CRPMessage::CRPMessage(uint16_t length, uint8_t op_code, uint32_t sender,
                       uint32_t receiver, char const *data)
    : length(length), op_code(op_code), sender(sender), receiver(receiver) {
  memcpy(this->data, data, length - 11);
}

uint16_t CRPMessage::get_length() { return length; }
uint32_t CRPMessage::get_sender() const { return sender; }
uint32_t CRPMessage::get_receiver() const { return receiver; }

const uint8_t *CRPMessage::get_data() const { return data; }
uint8_t CRPMessage::get_op_code() const { return op_code; }

int CRPMessage::unmarshal(const char *buf, int buflen) {
  length = ntohs(*((uint16_t *)buf));
  std::cout << "unmarshal length: " << length << std::endl;
  if (buflen < length) {
    return -length;
  } else {
    op_code = buf[2];
    sender = ntohl(*(uint32_t *)(buf + 3));
    receiver = ntohl(*(uint32_t *)(buf + 7));
    memcpy(data, buf + 11, length - 11);
  }
  return length;
}

int CRPMessage::marshal(char *buf, int buflen) {
  if (length > buflen) {
    return 0;
  }
  *((uint16_t *)buf) = htons(length);
  *((uint8_t *)(buf + 2)) = op_code;
  *((uint32_t *)(buf + 3)) = htonl(sender);
  *((uint32_t *)(buf + 7)) = htonl(receiver);
  memcpy(buf + 11, data, length - 11);

  return length;
}

void CRPMessage::DEBUG() {
  printf("{\n\tlength: %d\n\top_code: %d\n\tsender: %d\n\treceiver: "
         "%d\n\tdata: %s\n}\n",
         length, (int)op_code, sender, receiver, data);
}

//---------------------CRP-------------------
CRP::CRP(int fd) : fd(fd), recv_pointer(0), send_pointer(0) {}

int CRP::receive(CRPMessage *message) {
  int n = recv(fd, recv_buf + recv_pointer, 4096 - recv_pointer,
               0); //如果接收成功，返回实际接收到的字节数。
  if ((n == 0 && recv_pointer == 0) || n == -1) {
    return -1;
  }

  if (n > 0) {
    recv_pointer += n;
  }

  std::cout << "recv: " << n << std::endl;

  if (recv_pointer < 2) {
    return 1;
  }

  int len = CRPMessage::peek_length(recv_buf);

  if (recv_pointer < len) {
    return 1;
  }

  message->unmarshal(recv_buf, 4096);

  if (recv_pointer > len) {
    memcpy(recv_buf, recv_buf + len, recv_pointer - len);
  }
  recv_pointer -= len;

  return 0;
}

int CRP::send(CRPMessage *msg) {
  int m = 0;
  if (msg != nullptr)
    m = msg->marshal(send_buf + send_pointer, 4096 - send_pointer);
  send_pointer += m;
  int len = ::send(fd, send_buf, send_pointer, 0);
  // std::cout<<"send to "<<fd<<" "<< len<<" bytes"<<std::endl;
  // std::cout<<"amount to send: "<<send_pointer<<std::endl;
  memcpy(send_buf, send_buf + len, send_pointer - len);
  send_pointer -= len;

  return m;
}

int CRP::get_send_pointer() { return send_pointer; }

int CRP::close() {
  recv_pointer = 0;
  send_pointer = 0;
  return ::close(fd);
}

int CRP::get_fd() { return fd; }

void CRP::set_fd(int fd) { this->fd = fd; }
} // namespace net
} // namespace chatroom
