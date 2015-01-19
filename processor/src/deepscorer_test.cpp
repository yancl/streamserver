#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>

#include "include/deep-scorer.h"
#include "include/deep-open-scorer.h"

const int BUF_SIZE = 2048;

void test_open_scorer(const char* mode, const char* conf, const char* data_file) {
  DeepOpenScorer *dos = DeepOpenScorerNew(mode);
  DeepOpenScorerStart(dos, conf);
  DeepOpenScorerSetQid(dos, "IS20001");
  FILE* fh2 = fopen(data_file, "rb");
  while (1)
  {
    short buf[BUF_SIZE];
    int out_size = fread(buf, sizeof(*buf), BUF_SIZE, fh2);
    if (out_size == 0) break;
    DeepOpenScorerProcessRaw(dos, (const short *)buf, out_size);
  }
  
  DeepOpenScorerEnd(dos);
  char const *txt = DeepOpenScorerJsonOutput(dos);
  printf("%s", txt);
  DeepOpenScorerDestroy(dos);
  fclose(fh2);
}

int main(int argc, char **argv) {
  if(argc != 4) {
    fprintf(stderr, "%s GMM|NNET config wav\n", argv[0]);
    exit(1);
  }
  for (int i = 0; i < 4; i++) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    test_open_scorer(argv[1],argv[2],argv[3]);

    gettimeofday(&end, NULL);
    unsigned long timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + (end.tv_usec - start.tv_usec)/1000;
    printf("cost %lld ms\n", timeuse);
  }
}
