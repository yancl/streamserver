#ifndef SCRIBE_CALLBACK_H
#define SCRIBE_CALLBACK_H
#include <string>
#include "block_q.h"
#include "callback_msg.h"

class Callback {
public:
  Callback(BlockQueue<CallbackMsg>* message_q_ptr,const std::string& callback_url_prefix="/notify");
  virtual ~Callback();

public:
  static void* threadStatic(void *this_ptr);

private:
  void run();
  void callback(const CallbackMsg& msg);
  std::string makeUrl(const std::string& host, int port, const std::string& prefix);

private:
  std::string _callback_url_prefix;  
  BlockQueue<CallbackMsg>*  _message_q_ptr;
};

#endif
