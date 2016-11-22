/*
Fox Maikovich
foma2537
9/24/16

http://www.geeksforgeeks.org/scansets-in-c/
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>

void printMenu();

#define BUFFER_LENGTH 256

int main()
{
    char input;
    static char message[BUFFER_LENGTH];
    size_t size = 100;
    int file = 0;
    int r = 0;

    file = open("/dev/simple_character_device", O_RDWR);

    while (input != 'e') {
	if (file < 0) {
		printf("Cannot open the specified device.\n");
	}

        printMenu();
        scanf("%c%*c", &input);

        switch (input) {
        case 'r':
            r = read(file, message, BUFFER_LENGTH);
            if(r) {
                printf("Data on the file: %s\n", message);
            }
            else {
                printf("Read doesn't work.\n");
            }
            break;
        case 'w':
            printf("Enter data you want to write to the device: ");
            
	    scanf("%[^\n]%*c", message);

            if(write(file, message, sizeof(message))) {
                printf("Your data has been written to the file.\n");
            }
	    else {
		printf("Your data could not be written to the file.\n");
	    }
            break;
	default:
	    break;
        }

    }

    return 0;
}

void printMenu() {
    printf("Press r to read from device\nPress w to write to device\nPress e to exit from the device\nPress anything else to keep reading or writing from the device\nEnter command: ");
}
