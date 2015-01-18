#ifndef SCRIBE_CALLBACK_MESSAGE_H
#define SCRIBE_CALLBACK_MESSAGE_H
#include <string>

struct CallbackMsg {
  CallbackMsg(){}
  CallbackMsg(const std::string& host, int port, const std::string& key, const std::string& msg):_host(host), _port(port), _key(key), _msg(msg){}
  CallbackMsg& operator = (const CallbackMsg& rhs) {
    this->_host = rhs._host;
    this->_port = rhs._port;
    this->_key  = rhs._key;
    this->_msg  = rhs._msg;
    return *this;
  }

  CallbackMsg(const CallbackMsg& rhs) {
    this->_host = rhs._host;
    this->_port = rhs._port;
    this->_key  = rhs._key;
    this->_msg  = rhs._msg;
  }

  std::string _host;  
  int _port;
  std::string _key;
  std::string _msg;
};

#endif
