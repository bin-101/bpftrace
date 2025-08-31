#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "libbpf-usdt/usdt.h"

static long myclock()
{
  char buffer[100];
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if (USDT_IS_ACTIVE(tracetest, testprobe)) {
    snprintf(buffer, sizeof(buffer), "USDT is active\n");
  } else {
    snprintf(buffer, sizeof(buffer), "USDT is inactive\n");
  }
  USDT_WITH_SEMA(tracetest, testprobe, tv.tv_sec, buffer);
  return tv.tv_sec;
}

int main()
{
  while (1) {
    myclock();
    // Sleep is necessary to not overflow perf buffer
    usleep(1000);
  }
  return 0;
}
