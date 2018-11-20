#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

void receive_input(int signo) {
  printf("game received interrupt");
}

int main(int argc, char *argv[]) {
  printf("Hello world! -- game");
  FILE* gamepad = fopen("/dev/gamepad", "rb");
  if (gamepad != NULL) printf("found file");
  // this part leads to an unhandled exception. dunno why lol
  /*  printf("registering function returned signal %d", signal(SIGIO, &receive_input));
  printf("registering ownership returned signal %d", fcntl(fileno(gamepad), F_SETOWN, getpid()));
  int oflags = fcntl(fileno(gamepad), F_GETFL);
  printf("oflags: %d", oflags);
  printf("setting FL returned signal %d", fcntl(fileno(gamepad), F_SETFL, oflags | FASYNC));
  while(1) {} // debug i hope */
  exit(EXIT_SUCCESS);
}
