#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	printf("Hello");
	FILE* gamepad = fopen("/dev/gamepad", "r");
	unsigned int buttons;
	fread(&buttons, sizeof(unsigned int), 1, gamepad);
	printf("Buttons %d read", buttons);

	exit(EXIT_SUCCESS);
}
