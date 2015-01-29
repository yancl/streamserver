#ifndef STREAM_SERVER_RESOURCE_H
#define STREAM_SERVER_RESOURCE_H
#include <string>

namespace deepscore {

struct Resource {
  Resource(const std::string& path, const std::string& type, const std::string& qid) {
    this->path = path;
    this->type = type;
    this->qid  = qid;
  }
  std::string path;
  std::string type;
  std::string qid;
};

extern const Resource* g_Resource;
}
#endif
