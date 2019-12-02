#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define MAX_LINE_LENGTH 1024


struct Headers {
    bool quoted[MAX_LINE_LENGTH];
    int size, nameColumn;
};

struct node* head = NULL;

struct node {
    char* name;
    int length;
    struct node* next;
};


char* substring(char* source, int start, int end) {
    /*
     *  Creates a substring from start to end (digit at end not included).
     */
    int i = 0;
    char* substr = malloc(end - start + 1);

    while (start + i < end) {
        substr[i] = source[start + i];
        i += 1;
    }

    substr[i] = '\0';
    return substr;
}


int parseHeaders(char* line, struct Headers* headers) {
    /*
     *  Parse 1st line in the csv. Return -1 if invalid header is recieved.
     */
    int index = 0, nameColumn = -1;
    int start = 0, end = 0;
    bool cont = true;
    char* token;
    char* name;
    char* titles[MAX_LINE_LENGTH];

    while (cont == true) {
        if (line[end] != ',' && line[end] != '\n') {
            end += 1;
            continue;
        }

        if (line[end] == '\n')
            cont = false;

        token = substring(line, start, end);

        if (token[0] == '\"') {
            if (strlen(token) >= 2 && token[strlen(token) - 1] == '\"') { // id, "name", tweet
                headers->quoted[index] = true;
                name = substring(token, 1, strlen(token) - 1);
                free(token);
            } else // id,",tweet || id, "name, tweet
                return -1;
        } else if (token[strlen(token) - 1] == '\"') { // id, name", tweet
            return -1;
        } else { // id, name, tweet
            headers->quoted[index] = false;
            name = token;
        }

        for (int i = 0; i < index; i++) {
            if (strcmp(titles[i], name) == 0) // this header already exists
                return -1;
        }

        if (strcmp(name, "name") == 0)
            nameColumn = index;

        fprintf(stderr, "header: %s\n", name);                                              // TODO: remove me later
        titles[index] = name;
        index += 1;
        end += 1;
        start = end;
    }

    for (int i = 0; i < index; i++)
        free(titles[i]);

    headers->size = index;
    headers->nameColumn = nameColumn;
    return nameColumn;
}


int parseBody(char* line, struct Headers* headers /*, Map* dictionary */) {
    /*
     *  Parse body of the csv. Return -1 if invalid body is recieved (header is quoted, but body is not etc.).
     */
     int index = 0;
     int start = 0, end = 0;
     char* token;
     char* name;
     bool cont = true;

     while (cont == true) {
        if (line[end] != ',' && line[end] != '\n') {
         end += 1;
         continue;
        }

        if (line[end] == '\n')
            cont = false;

        if (index > headers->size - 1) // more columns than headers
            return -1;

        token = substring(line, start, end);

        if (headers->quoted[index] == true) {
            if (strlen(token) >= 2 && token[0] == '\"' && token[strlen(token) - 1] == '\"') {
                if (index == headers->nameColumn) {
                    name = substring(token, 1, strlen(token) - 1);
                    free(token);
                }
            } else
                return -1;
        } else {
            if (index == headers->nameColumn)
                name = token;
        }

        if (index == headers->nameColumn) {
            fprintf(stderr, "name: %s\n", name);                                        // TODO: remove me later

            if (head == NULL) {
                head = (struct node*)malloc(sizeof(struct node));
                head->name = malloc(strlen(name));
                strcpy(head->name, name);
                head->length = 1;
                head->next = NULL;
            }
            else if (strcmp(head->name, name) == 0) {
                head->length += 1;
            }
            else {
                struct node* curr = head->next;
                struct node* prev = head;
                int found = -1;
                while (curr != NULL) {
                    if (strcmp(curr->name, name) == 0) {
                        curr->length += 1;
                        found = 1;
                        break;
                    }
                    prev = curr;
                    curr = curr->next;
                }
                if (found == -1) {
                    curr = (struct node*)malloc(sizeof(struct node));
                    curr->length = 1;
                    prev->next = curr;
                    curr->next = NULL;
                    curr->name = malloc(strlen(name));
                    strcpy(curr->name, name);
                }
            }

            free(name);
        }

        index += 1;
        end += 1;
        start = end;
    }

    if (index != headers->size) // less columns than headers
        return -1;

    return 0;
}


void insertSort(struct node** headSort, struct node* input) {
    struct node* curr;
    if (*headSort == NULL || (*headSort)->length <= input->length) {
        input->next = *headSort;
        *headSort = input;
    }
    else {
        curr = *headSort;
        while(curr->next!=NULL && curr->next->length>input->length){
            curr = curr->next;
        }
        input->next = curr->next;
        curr->next = input;
    }
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


            // struct node* headSort = NULL;
            // struct node* currentSort = head;
            // while (currentSort != NULL) {
            //     struct node* nextSort = currentSort->next;
            //     insertSort(&headSort, currentSort);
            //     currentSort = nextSort;
            // }

            struct node* current = head;

            while (current != NULL){
                printf("name: %s | length: %d\n", current-> name, current->length);
                current = current->next;
            }
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
