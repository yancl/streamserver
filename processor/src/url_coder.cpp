#include "include/url_coder.h"

int deepscore::URLCoder::encode(const std::string& src, std::string& dst) {
  CURL *curl = curl_easy_init();
  if(curl) {
    char *escaped = curl_easy_escape(curl, (char*)src.c_str(), src.length());
    if(escaped) {
      dst = std::string(escaped);
      curl_free(escaped);
      return 0;
    }
    curl_easy_cleanup(curl);
  }
  return -1;
}

int deepscore::URLCoder::decode(const std::string& src, std::string& dst) {
  CURL *curl = curl_easy_init();
  if(curl) {
    int len = 0;
    char *unescaped = curl_easy_unescape(curl, (char*)src.c_str(), src.length(), &len);
    if(unescaped) {
      dst = std::string(unescaped);
      curl_free(unescaped);
      return 0;
    }
    curl_easy_cleanup(curl);
  }
  return -1;
}
