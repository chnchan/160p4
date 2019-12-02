#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define MAX_LINE_LENGTH 1024


struct Headers {
    bool quoted[MAX_LINE_LENGTH];
    int size, nameColumn;
};

int parseHeaders(char* line, struct Headers* headers) {
    /*
     *  Parse 1st line in the csv. Return -1 if invalid header is recieved.
     */

    int index = 0, nameColumn = -1;
    char* titles[MAX_LINE_LENGTH];
    char* token = strtok(line, ",");

    while (token) {
        if (token[strlen(token) - 1] == '\n') // remove newline from the last header item
            token[strlen(token) - 1] = '\0';

        for (int i = 0; i < index; i++) {
            if (strcmp(titles[i], token) == 0) // this header already exists
                return -1;
        }

        if (token[0] == '\"') {
            if (strlen(token) >= 2 && token[strlen(token) - 1] == '\"') { // id, "name", tweet
                headers->quoted[index] = true;
                titles[index] = malloc(strlen(token) - 1);
                strncpy(titles[index], token + 1, strlen(token) - 2);
            } else // id,",tweet || id, "name, tweet
                return -1;
        } else if (token[strlen(token) - 1] == '\"') { // id, name", tweet
            return -1;
        } else { // id, name, tweet
            headers->quoted[index] = false;
            titles[index] = malloc(strlen(token) - 1);
            strcpy(titles[index], token);
        }

        fprintf(stderr, "name: %s\n", titles[index]);             // TODO: remove me later
        if (strcmp(titles[index], "name") == 0)
            headers->nameColumn = index;

        index += 1;
        token = strtok(NULL, ",");
    }

    for (int i = 0; i < index; i++)
        free(titles[i]);

    headers->size = index;
    return 0;
}

int parseBody(char* line, struct Headers* headers /*, Map* dictionary */) {
    /*
     *  Parse body of the csv. Return -1 if invalid body is recieved (header is quoted, but body is not etc.).
     */

    int index = 0;
    char* token = strtok(line, ",");

    while (token) {
        if (index > headers->size - 1) // more columns than headers
            return -1;

        if (token[strlen(token) - 1] == '\n') // remove newline from the last header item
            token[strlen(token) - 1] = '\0';

        if (headers->quoted[index] == true) {
            if (strlen(token) >= 2 && token[0] == '\"' && token[strlen(token) - 1] == '\"') {
                if (index == headers->nameColumn) {
                    char* name = malloc(strlen(token) - 1);
                    strncpy(name, token + 1, strlen(token) - 2);
                    // dictionary[substring] += 1;
                }
            } else
                return -1;
        } else {
            if (index == headers->nameColumn) {
                // dictionary[token] += 1;
            }
        }

        index += 1;
        token = strtok(NULL, ",");
    }

    if (index != headers->size - 1) // less columns than headers
        return -1;

    return 0;
}

int main(int argc, char** argv) {
    char line[MAX_LINE_LENGTH];
    FILE* inf = fopen(argv[1], "r");
    // struct headerItem headers[MAX_LINE_LENGTH]; // or can use a linked list
    struct Headers headers;
    // bool headers[MAX_LINE_LENGTH];

    if (inf == NULL)
        printf("File does not exist\n");
    else {
        fgets(line, MAX_LINE_LENGTH, inf);

        if (parseHeaders(line, &headers) == -1)
            printf("Invalid Input Format\n");
        else {
            while (fgets(line, MAX_LINE_LENGTH, inf)) {
                if (parseBody(line, &headers) == -1)
                    printf("Invalid Input Format\n");
            }

            // print result
        }
    }

    return 0;
}


/* Overview:

1.  store header infos (can be as many header as possible within MAX_LINE_LENGTH limit).
    If the header item is quoted, all of items in that column needs to be quoted as well. Otherwise,
    it is considered invalid (piazza 295).

    Ex.
    id, name, tweet is valid
    id, name, "name", tweet is invalid
    "MyName"" is valid and will return MyName"

2.  read body without crashing (even invalid inputs). Make sure if the column header is quoted,
    the content is also quoted.

3.  write a dictionary / hash table to store each tweeter's tweet count

    Ex.
    dict["Prem"] = 1

*/


/* Test cases:

id,,name,,tweet     - should be invalid
id,,name,tweeter    - should be valid

*/
