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
    if (substr == NULL)
        return NULL;

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
        if (token == NULL)
            return -1;

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

        if (name == NULL)
            return -1;

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
     char* token = NULL;
     char* name = NULL;
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
        if (token == NULL)
            return -1;

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
            if (name == NULL)
                return -1;

            fprintf(stderr, "name: %s\n", name);                                        // TODO: remove me later
            struct node *curr = head, *prev = NULL;

            while (curr != NULL) {
                if (strcmp(curr->name, name) == 0)
                    break;
                else {
                    prev = curr;
                    curr = curr->next;
                }
            }

            if (curr != NULL)
                curr->length += 1;
            else { // no node with name found in the linked list
                if (prev == NULL) { // head is not initialized
                    head = (struct node*)malloc(sizeof(struct node));
                    if (head == NULL)
                        return -1;

                    head->name = malloc(strlen(name));
                    if (head->name == NULL)
                        return -1;

                    strcpy(head->name, name);
                    head->length = 1;
                    head->next = NULL;
                } else { // initialized a new tail node
                    curr = (struct node*)malloc(sizeof(struct node));
                    if (curr == NULL)
                        return -1;

                    curr->length = 1;
                    prev->next = curr;
                    curr->next = NULL;
                    curr->name = malloc(strlen(name));
                    if (curr->name == NULL)
                        return -1;

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
    struct Headers headers;

    if (inf == NULL) {
        printf("Invalid File / File Path\n");
        return -1;
    }
    else {
        fgets(line, MAX_LINE_LENGTH, inf);

        if (strlen(line) >= MAX_LINE_LENGTH && line[strlen(line) - 1] != '\n') {
            printf("Invalid Input Format: line exceeded MAX_LINE_LENGTH\n");
            return -1;
        }

        if (parseHeaders(line, &headers) == -1){
            printf("Invalid Input Format: invalid headers\n");
            return -1;
        }
        else {
            int totalLines = 1;
            bool valid = true;
            while (fgets(line, MAX_LINE_LENGTH, inf)) {
                if (strlen(line) >= MAX_LINE_LENGTH && line[strlen(line) - 1] != '\n') {
                    printf("Invalid Input Format: line exceeded MAX_LINE_LENGTH\n");
                    return -1;
                }

                if (totalLines >= 20000) {
                    printf("Invalid Input Format: input file exceeded 20,000 lines\n");
                    return -1;
                }

                if (parseBody(line, &headers) == -1){
                    printf("Invalid Input Formats: invalid content\n");
                    return -1;
                }

                totalLines += 1;
            }

            int count = 0;
            struct node* headSort = NULL;
            struct node* currentSort = head;
            while (currentSort != NULL) {
                struct node* nextSort = currentSort->next;
                insertSort(&headSort, currentSort);
                currentSort = nextSort;
            }

            struct node* current = headSort;
            while (current != NULL && count < 10){
                printf("%s: %d\n", current-> name, current->length);
                current = current->next;
                count += 1;
            }
        }
    }

    return 0;
}
