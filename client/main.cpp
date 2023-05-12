#include "../public/crp.hpp"
#include "../public/socket.hpp"
#include "fcntl.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sys/_pthread/_pthread_t.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

using namespace chatroom::net;

void receive_messages(CRP *crp) {
  int file_len = 0;
  int fd;
  while (true) {
    CRPMessage message;
    int status = crp->receive(&message);
    if (status < 0) {
      printf("Disconnected from server.\n");
      break;
    } else if (status == 0) {
      switch (message.get_op_code()) {
      case CHAT:
        printf("user %d: %s  \n", message.get_sender(), message.get_data());
        break;
      case LOGOUT:
        crp->close();
        break;
      case FILE_START:
        fd = open((const char *)message.get_data(), O_CREAT | O_WRONLY, 777);
        if (fd == -1) {
          printf("open fail\n");
        }
        break;
      case FILE_MID:
        write(fd, message.get_data(), message.get_length() - 11);
        break;
      case FILE_END:
        write(fd, message.get_data(), message.get_length() - 11);
        close(fd);
        break;
      }
    }
  }
}

void send_messages(CRP *crp, uint32_t user_id) {
  while (true) {
    std::string message_text;
    uint32_t receiver;
    int chat;

    printf("user ID: ");
    scanf("%d", &receiver);

    printf("chat: ");
    scanf("%d", &chat);

    if (chat == 0) {
      printf("Enter your message: ");
      std::cin.ignore();
      std::getline(std::cin, message_text);

      CRPMessage message(11 + message_text.length() + 1, OP_CODE::CHAT, user_id,
                         receiver, message_text.c_str());
      crp->send(&message);
    } else if (chat == -1) {
      CRPMessage logout_message(11 + 6, OP_CODE::LOGOUT, user_id, 0, "LOGOUT");
      crp->send(&logout_message);
      break;
    } else {
      printf("Enter your file path: ");
      std::cin.ignore();
      std::getline(std::cin, message_text);
      struct stat s;

      int fd = open(message_text.c_str(), O_RDONLY);
      if (fd == -1) {
        printf("open fail\n");
        continue;
      }
      char buffer[4096];
      int l = message_text.find_last_of('/');
      std::string fname = std::string("./save/").append(
          message_text.substr(l + 1, message_text.length() - l));

      CRPMessage message(11 + fname.length() + 1, OP_CODE::FILE_START, user_id,
                         receiver, fname.c_str());
      crp->send(&message);

      while (true) {
        int n = read(fd, buffer, 4096 - 11);
        if (n == 4096 - 11) {
          CRPMessage message(11 + n, OP_CODE::FILE_MID, user_id, receiver,
                             buffer);
          crp->send(&message);
        } else if (n != 0) {
          CRPMessage message(11 + n, OP_CODE::FILE_END, user_id, receiver,
                             buffer);
          crp->send(&message);
        } else {
          break;
        }
      }
    }
  }
}

void receive_messages_wrapper(chatroom::net::CRP *crp_ptr) {
  receive_messages(crp_ptr);
}

int main() {
  std::string server_address;
  uint16_t server_port;
  uint32_t user_id;
  char password[1024];

  std::cout << "Enter server address: ";
  std::cin >> server_address;

  std::cout << "Enter server port: ";
  std::cin >> server_port;

  SocketStreamClient client(server_address.c_str(), server_port);

  if (client.connect() != 0) {
    std::cerr << "Failed to connect to server." << std::endl;
    return 1;
  }

  CRP crp(client.get_sock_fd());

  std::cout << "Enter your user ID: ";
  std::cin >> user_id;

  std::cout << "Enter your password: ";
  std::cin >> password;

  // Send login message to server
  CRPMessage login_message(11 + strlen(password) + 1, OP_CODE::LOGIN, user_id,
                           0, password);
  // login_message.DEBUG();
  crp.send(&login_message);

  std::thread receive_messages_thread(receive_messages_wrapper, &crp);

  std::thread send_messages_thread(send_messages, &crp, user_id);

  send_messages_thread.join();
  receive_messages_thread.join();
  crp.close();

  return 0;
}
