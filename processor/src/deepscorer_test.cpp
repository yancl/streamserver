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
#include <iostream>

#include "../include/deep-scorer.h"
#include "../include/deep-open-scorer.h"
//#include "../utils/gettime.h"


int main(int argc, char **argv) {
  if(argc != 5)
  {
    fprintf(stderr, "%s GMM|NNET config wavlist outdir\n", argv[0]);
    exit(1);
  }
  short buf[2048];
  size_t out_size;
  char *wavlist = argv[3];
  char *outdir = argv[4];
  for (int i = 0; i < 3; i++) {
    struct timeval start, end;
    FILE* fh = fopen(wavlist, "rb");
    printf("wavlist=%s\n",wavlist);
    FILE* out = fopen("chn.json", "wa");
    char lineptr[1024];

    gettimeofday(&start, NULL);

    while(fgets(lineptr, 1024, fh) != NULL)
    {
      size_t ln = strlen(lineptr) - 1;
      if (lineptr[ln] == '\n')
        lineptr[ln] = '\0';
      puts(lineptr);
      DeepOpenScorer *dos = DeepOpenScorerNew(argv[1]);
      DeepOpenScorerStart(dos, argv[2]);
      DeepOpenScorerSetQid(dos, "IS20001");
      FILE* fh2 = fopen(lineptr, "rb");
      while (1)
      {
        out_size = fread(buf, sizeof(*buf), 2048, fh2);
        if (out_size == 0) break;
        DeepOpenScorerProcessRaw(dos, (const short *)buf, out_size);
      }

      DeepOpenScorerEnd(dos);
      char const *txt = DeepOpenScorerJsonOutput(dos);
      fprintf(out, "%s", txt);
      DeepOpenScorerDestroy(dos);
      fclose(fh2);
    }
    fclose(fh);
    fclose(out);

    gettimeofday(&end, NULL);
    unsigned long timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + (end.tv_usec - start.tv_usec)/1000;
    std::cout << "cost:(" << timeuse << ")ms" << std::endl;
  }
}
