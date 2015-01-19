#ifndef STREAM_SERVER_URLCODE_H
#define STREAM_SERVER_URLCODE_H
#include <string>
#include <curl/curl.h>

namespace deepscore {

class URLCoder {
public:
  int encode(const std::string& src, std::string& dst);
  int decode(const std::string& src, std::string& dst);
};

}
#endif
