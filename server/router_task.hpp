#pragma once

#include "crp.hpp"
#include "iostream"
#include "server.hpp"
#include "thread_pool/task.hpp"
#include <pthread.h>
namespace chatroom {
namespace net {

struct RouterTaskData {
  CRPMessage *message_ptr;
  Server *server_ptr;
  int fd;
};

class RouterTask : public thread_pool::Task {
public:
  void exec(void *router_task_data) override {

    chatroom::net::CRPMessage *message =
        ((RouterTaskData *)router_task_data)->message_ptr;
    chatroom::net::Server *server =
        ((RouterTaskData *)router_task_data)->server_ptr;
    int fd = ((RouterTaskData *)router_task_data)->fd;

    pthread_rwlock_rdlock(server->get_sender_fd_rwlock());
    int exist = server->get_sender_fd()->count(message->get_receiver());
    pthread_rwlock_unlock(server->get_sender_fd_rwlock());

    if (exist) {
      pthread_rwlock_rdlock(server->get_sender_fd_rwlock());
      int fd = server->get_sender_fd()->at(message->get_receiver());
      pthread_rwlock_unlock(server->get_sender_fd_rwlock());

      std::cout<<"router to fd: "<<fd<<std::endl;

      pthread_mutex_lock(&server->get_message_queue_mutex()[fd]);
      server->get_message_queue()[fd].push(message);
      pthread_mutex_unlock(&server->get_message_queue_mutex()[fd]);

      pthread_mutex_lock(server->get_write_set_mutex());
      FD_SET(fd ,server->get_write_set());
      pthread_mutex_unlock(server->get_write_set_mutex());
    } else {
    }
  }

  RouterTask(void *args) : thread_pool::Task(args){};
};

} // namespace net
} // namespace chatroom