#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include "../utils/timer.c"


typedef struct {
    int critical;
    int error;
    int failed_login;
} Result;

typedef struct {
    const char *filename;
    off_t start;
    off_t end;
    int skip;
    Result *result;
} ThreadArgs;

typedef struct {
    const char *filename;
    int n;
    Result *result;
} FileArgs;

int findLineEnd(int startPos, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: No file found in directory");
    } else {
        char ch = fgetc(fp);
        int i = 0;
        fseek(fp, startPos, SEEK_SET);
        while(ch != '\n' && ch != EOF) {
            ch = fgetc(fp);
            i++;
        }
        fclose(fp);
        return startPos + i;
    }
    fclose(fp);
    return NULL;
}

void *analyze_block(void *args) {
    ThreadArgs *data = (ThreadArgs *)args;
    int fd = open(data->filename, O_RDONLY);
    if (fd < 0) pthread_exit(NULL);

    char buffer[data->end - data->start + 1];
    Result local = {0, 0, 0};

    if (!data->skip) {
        off_t bytes_read = 0;
        while (1) {
            ssize_t r = pread(fd, buffer, data->end - data->start - bytes_read, data->start + bytes_read);
            if (r <= 0) break;
            buffer[r] = '\0';

            char *found = strstr(buffer, "CRITICAL");
            if (found) {
                local.critical++;
                bytes_read += (found - buffer) + strlen("CRITICAL");
            } else break;
        }

        bytes_read = 0;
        while (1) {
            ssize_t r = pread(fd, buffer, data->end - data->start - bytes_read, data->start + bytes_read);
            if (r <= 0) break;
            buffer[r] = '\0';

            char *found = strstr(buffer, "ERROR");
            if (found) {
                local.error++;
                bytes_read += (found - buffer) + strlen("ERROR");
            } else break;
        }

        bytes_read = 0;
        while (1) {
            ssize_t r = pread(fd, buffer, data->end - data->start - bytes_read, data->start + bytes_read);
            if (r <= 0) break;
            buffer[r] = '\0';

            char *found = strstr(buffer, "FAILED_LOGIN");
            if (found) {
                local.failed_login++;
                bytes_read += (found - buffer) + strlen("FAILED_LOGIN");
            } else break;
        }
    }

    close(fd);

    data->result->critical += local.critical;
    data->result->error += local.error;
    data->result->failed_login += local.failed_login;

    pthread_exit(NULL);
}


void *process_file_thread(void *args) {
    FileArgs *fileArgs = (FileArgs *)args;
    process_file(fileArgs->filename, fileArgs->n, fileArgs->result);
    pthread_exit(NULL);
}

void process_file(const char *filename, int n, Result *result) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return;

    off_t file_size = lseek(fd, 0, SEEK_END);
    off_t block_size = file_size / n;
    close(fd);

    pthread_t threads[n];
    ThreadArgs args[n];

    int lineStart = 0, lineEnd = 0;
    off_t lastBlock = 0;

    for (int i = 0; i < n; i++) {
        off_t blockEnd = (i == n - 1) ? file_size : lastBlock + block_size;
        lineStart = lineEnd;
        lineEnd = findLineEnd(blockEnd, filename);
        lastBlock = blockEnd;

        args[i] = (ThreadArgs){
            .filename = filename,
            .start = lineStart,
            .end = lineEnd,
            .skip = (lineEnd <= lineStart),
            .result = result,
        };

        pthread_create(&threads[i], NULL, analyze_block, &args[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }
}

void section3(int *arg) {
    int n = *(int *)arg;

    Result res1 = {0, 0, 0};
    Result res2 = {0, 0, 0};

    pthread_t fileThreads[2];
    FileArgs args1 = { "logs.txt", n, &res1 };
    FileArgs args2 = { "logs_2.txt", n, &res2 };

    pthread_create(&fileThreads[0], NULL, process_file_thread, &args1);
    pthread_create(&fileThreads[1], NULL, process_file_thread, &args2);

    pthread_join(fileThreads[0], NULL);
    pthread_join(fileThreads[1], NULL);
    
    FILE *out = fopen("RESULT_THREADS.txt", "w");
    fprintf(out, "logs.txt: CRITICAL=%d ERROR=%d FAILED LOGIN=%d\n", res1.critical, res1.error, res1.failed_login);
    fprintf(out, "logs_2.txt: CRITICAL=%d ERROR=%d FAILED LOGIN=%d\n", res2.critical, res2.error, res2.failed_login);
    fclose(out);

    printf("Main thread finished.\n");
    return 0;
}




int main() {
    int n;
    printf("Entrez le nombre de blocs : ");
    scanf("%d", &n);

    double time = measure_time(section3, &n);
    printf("Temps d'exécution avec processus : %.6f secondes\n", time);
}

