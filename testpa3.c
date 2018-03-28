#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 1024

int main(void)
{
    char inputChoice, c, receiveLength;
    int inputDevice, outputDevice, stringLength, returnValue;
    char *stringToSend, *messageLength, *message;
    char *ptr;

    stringToSend  = NULL;
    messageLength = NULL;
    message       = NULL;

    stringToSend  = malloc(sizeof(char) * BUFFER_LENGTH);
    messageLength = malloc(sizeof(char) * BUFFER_LENGTH);

    inputDevice  = open("/dev/pa3_input", O_RDWR);
    outputDevice = open("/dev/pa3_output", O_RDWR);

    if (inputDevice < 0)
    {
        perror("TESTPA3: Failed to open the input device.\n");

        return errno;
    }

    if (outputDevice < 0)
    {
        perror("TESTPA3: Failed to open the output device.\n");

        return errno;
    }

    printf("TESTPA3: Type W to write.\n");
    printf("TESTPA3: Type R to read.\n");
    printf("TESTPA3: Type Q to quit.\n");

    do {
        printf("TESTPA3: What do you want to do? ");
        inputChoice = getchar();

        // Clear the stdin buffer
        while ((c = getchar()) != '\n' && c != EOF);

        switch (inputChoice)
        {
            case 'w':
            case 'W':
                printf("TESTPA3: Enter a string to write:\n");
                printf("TESTPA3: ");

                fgets(stringToSend, BUFFER_LENGTH, stdin);
                strtok(stringToSend, "\n");
                stringLength = strlen(stringToSend) >= BUFFER_LENGTH ? BUFFER_LENGTH : strlen(stringToSend);

                returnValue = write(inputDevice, stringToSend, stringLength);

                break;
            case 'r':
            case 'R':
                message = calloc(BUFFER_LENGTH, sizeof(char));

                printf("TESTPA3: How many bytes would you like to read?\n");
                printf("TESTPA3: %s\n", message);
                printf("TESTPA3: ");

                fgets(messageLength, BUFFER_LENGTH, stdin);
                receiveLength = atoi(messageLength);

                returnValue = read(outputDevice, message, receiveLength);

                printf("TESTPA3: %s\n", message);

                free(message);
                message = NULL;

                break;
            case 'q':
            case 'Q':
                printf("TESTPA3: Bye-bye.\n");
                break;
            default:
                printf("TESTPA3: Invalid input option.\n");
                break;
        }
    } while (inputChoice != 'Q' && inputChoice != 'q');

    // free(stringToSend);
    // free(messageLength);

    if (message != NULL)
    {
        free(message);
    }

    return 0;
}
