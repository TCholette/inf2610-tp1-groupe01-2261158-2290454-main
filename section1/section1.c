#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    compterMots();
    return 0;
}

void compterMots() {
    FILE *fp = fopen("../utils/mots.txt", "r");
    if (fp == NULL) {

        printf("Error: No file found in directory");
    } else { 
        int debounce = 1;
        int sum = 0;
        char ch = fgetc(fp);
        while(ch != EOF) {
            ch = fgetc(fp);
            if (ch == EOF) {
                break;
            }
            if (isspace(ch)) {
                debounce = 1;
            } else {
                if (debounce == 1) {
                    sum++;
                }
                debounce = 0;
            }
        }
        FILE *fp2 = fopen("section2_2.txt", "w");
        char buffer[16] = {0};
        sprintf(buffer, "%i", sum);
        fputs(buffer, fp2);
    }
    return;
}
