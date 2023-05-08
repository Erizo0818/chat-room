#include "../public/crp.hpp"
#include "../public/socket.hpp"
#include <cstring>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>
#include <unistd.h>
#include <string>


using namespace chatroom::net;

void receive_messages(CRP *crp) {
  while (true) {
    CRPMessage message;
    int status = crp->receive(&message);
    if (status < 0) {
      std::cout << "Disconnected from server." << std::endl;
      break;
    } else if (status == 0) {
        std::cout << "Received message from user " << message.get_sender() << ": " << message.get_data() << std::endl;
    }
  }
}

void send_messages(CRP *crp, uint32_t user_id) {
  while (true) {
    std::string message_text;
    uint32_t receiver;

    std::cout << "Enter receiver user ID: ";
    std::cin >> receiver;

    std::cout << "Enter your message: ";
    std::cin.ignore();
    std::getline(std::cin, message_text);

    if (message_text == "/quit") {
      CRPMessage logout_message(11 + 6, OP_CODE::LOGOUT, user_id, 0, "LOGOUT");
      crp->send(&logout_message);
      break;
    }


    CRPMessage message(11 + message_text.length()+1, OP_CODE::CHAT, user_id, receiver, message_text.c_str());
    crp->send(&message);
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

  std::cout << "Enter your user ID: ";
  std::cin >> password;

  // Send login message to server
  CRPMessage login_message(11 + strlen(password) + 1, OP_CODE::LOGIN, user_id, 0, password);
  login_message.DEBUG();
  crp.send(&login_message);

  std::thread receive_messages_thread(receive_messages_wrapper, &crp);

  std::thread send_messages_thread(send_messages, &crp, user_id);

  send_messages_thread.join();
  receive_messages_thread.join();
  crp.close();

  return 0;
}
