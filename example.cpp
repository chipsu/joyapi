#include <stdio.h>
#include "joyapi.h"

using namespace JoyAPI;

char chr(int i) {
	if(i < 10) return '0' + i;
	if(i < 36) return 'a' + i - 10;
	if(i < 62) return 'A' + i - 36;
	return '.';
}

int main(int argc, char **argv) {
    SimpleDevice dev;
    if(!dev.Open()) {
        fprintf(stderr, "Device is not open\n");
        return -1;
    }

	printf("Joystick detected: %s\n\t%d axes\n\t%d buttons\n\n", dev.GetName(), dev.GetNumAxes(), dev.GetNumButtons());

	// FIXME: Formatting
    for(int i = 0; i < dev.GetNumAxes() && i < 15; ++i) {
        printf("axis%02d  ", i + 1);
    }

    for(int i = 0; i < dev.GetNumButtons(); ++i) {
        printf("%c", chr(i));
    }

    printf("\n");

    while(dev.Poll()) {
        for(int i = 0; i < dev.GetNumAxes() && i < 15; ++i) {
			printf("%6d  ", dev.GetAxis(i));
        }
			
		for(int x = 0; x < dev.GetNumButtons(); ++x) {
			printf("%d", dev.GetButton(x));
        }

		printf("  \r");
		fflush(stdout);
    }

    dev.Close();

    return 0;
}

