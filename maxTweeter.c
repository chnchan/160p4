#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

struct headerItem {
    char* name;
    bool quoted;
};


int parseHeaders(char* line, struct headerItem* headers) {
    /*
     *  Parse 1st line in the csv. Return -1 if invalid header is recieved.
     */

    int index = 0, nameColumn = -1;
    char* token = strtok(line, ",");

    while (token) {
        if (token[strlen(token) - 1] == '\n') // remove newline from the last header item
            token[strlen(token) - 1] = '\0';

        for (int i = 0; i < index; i++) {
            if (strcmp(headers[i].name, token) == 0)
                return -1;
        }

        if (token[0] == '\"') {
            if (strlen(token) >= 2 && token[strlen(token) - 1] == '\"') { // id, "name", tweet
                headers[index].quoted = true;
                headers[index].name = malloc(strlen(token) - 1);
                strncpy(headers[index].name, token + 1, strlen(token) - 2);
            } else // id,",tweet || id, "name, tweet
                return -1;
        } else if (token[strlen(token) - 1] == '\"') { // id, name", tweet
            return -1;
        } else { // id, name, tweet
            headers[index].quoted = false;
            headers[index].name = malloc(strlen(token) + 1);
            strcpy(headers[index].name, token);
        }

        fprintf(stderr, "name: %s\n", headers[index].name);             // TODO: remove me later
        if (strcmp(headers[index].name, "name") == 0)
            nameColumn = index;

        index += 1;
        token = strtok(NULL, ",");
    }

    return nameColumn;
}


int main(int argc, char** argv) {
    int nameColumn = -1;
    char line[MAX_LINE_LENGTH];
    FILE* inf = fopen(argv[1], "r");
    struct headerItem headers[MAX_LINE_LENGTH]; // or can use a linked list

    if (inf == NULL)
        printf("File does not exist\n");
    else {
        fgets(line, MAX_LINE_LENGTH, inf);

        if ((nameColumn = parseHeaders(line, headers)) == -1) {
            printf("Invalid Input Format\n");
        } else {
            // parse body
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
