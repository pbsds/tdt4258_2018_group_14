#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

FILE* gamepad;
int buttons;

void receive_input(int signo) {
  fread(&buttons, sizeof(buttons), 1, gamepad);
  printf("Buttons pressed: %d", buttons);
  return;
}

int main(int argc, char *argv[]) {
  gamepad = fopen("/dev/gamepad", "rb");
  if (gamepad != NULL) {
    printf("found file /dev/gamepad");
  }
  int fno = fileno(gamepad);
  
  //set up sigaction
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = receive_input; // set function to run when signal is received
  action.sa_flags = 0; // no relevant flags
  sigaction(SIGIO, &action, NULL); // run interrupt on SIGIO

  // Set ownership of gamepad to this program.
  if (fcntl(fno, F_SETOWN, getpid()) == -1) {
    printf("Error in setting ownership of gamepad.");
    return EXIT_FAILURE;
  }
  if (fcntl(fno, F_SETFL, fcntl(fno, F_GETFL) | FASYNC) == -1) {
    printf("Error in setting FASYNC flag in gamepad.");
    return EXIT_FAILURE;
  }
  while (1) {
    printf("successss");
    sleep(10000);
  }
  return EXIT_SUCCESS;
}
