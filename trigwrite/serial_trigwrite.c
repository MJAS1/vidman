/*
 * Fiddle the handshake lines of a serial port
 *
 * Lauri Parkkonen, 2013
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[])
{
  int   fd = -1;
  char *port = argv[1];
  char *me = argv[0];
  int   lines_code[] = {0, TIOCM_DTR, TIOCM_RTS, TIOCM_DTR | TIOCM_RTS};
  char *lines_name[] = {"---", "DTR", "RTS", "DTR+RTS"};
  int   i;

  /*
   * DTR - Data terminal ready - pin 4 on the D9 connector
   * RTS - Request to send - pin 7 on the D9 connector
   */
  
  if (port == NULL)
  {
    fprintf(stderr, "%s: Specify the device name of the port as the first argument\n", me);
    exit(1);
  }
  
  if ((fd = open(port, O_RDWR)) < 1)
  {
    fprintf(stderr, "%s: Cannot open port %s: %s\n", me, port, strerror(errno));
    exit(2);
  }
  
  for (i = 0;;) 
  {
    i = i + 1;
    if (i > 3) i = 0;
    
    printf("Setting modem control lines as %s\n", lines_name[i]);
    
    if (ioctl(fd, TIOCMSET, lines_code + i) == -1)
    {
      fprintf(stderr, "%s: ioctl call failed on %s: %s\n", me, port, strerror(errno));
      exit(3);
    }
    usleep(1500*1000);
  }
}
