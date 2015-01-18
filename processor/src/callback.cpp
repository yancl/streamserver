#include "include/http_client.h"
#include "include/common.h"
#include "include/callback.h"

void* deepscore::Callback::threadStatic(void *this_ptr) {
  Callback *callback_ptr = (Callback*)this_ptr;
  callback_ptr->run();
  return NULL;
}

deepscore::Callback::Callback(BlockQueue<CallbackMsg>* message_q_ptr,const std::string& callback_url_prefix):
  _message_q_ptr(message_q_ptr), _callback_url_prefix(callback_url_prefix) {}
deepscore::Callback::~Callback() {}

void deepscore::Callback::run() {
  bool stop = false;
  while (!stop) {
    CallbackMsg msg;
    _message_q_ptr->wait_and_pop(msg);
    this->callback(msg);
  }
}

void deepscore::Callback::callback(const CallbackMsg& msg) {
  std::string url = makeUrl(msg._host, msg._port, _callback_url_prefix);

  CHttpClient http_client;
  std::string rsp;
  int rv = http_client.Get(url, rsp); 
  if (rv != 0) {
    LOG(ERROR) << "callback failed for key:" << msg._key << "rv:" << rv;
    return;
  }
  LOG(INFO) << "callback ok for key:" << msg._key;
}


std::string deepscore::Callback::makeUrl(const std::string& host, int port, const std::string& prefix) {
  std::ostringstream oss;
  oss << "http://" << host << ":" << port << "/" << prefix;
  return oss.str(); 
}
