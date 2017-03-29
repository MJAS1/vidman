#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
  char *me = argv[0];

  if(ioperm(12304, 1, 1))
  {
    fprintf(stderr, "%s: Cannot open port %d: %s\n", me, 12304, strerror(errno));
    exit(2);
  }


  for(int i = 0; i < 256; i++)
  {

    printf("Setting outb %d\n", i);
    outb(i, 12304);

    usleep(1500*1000);
  }
 
}
	
	
