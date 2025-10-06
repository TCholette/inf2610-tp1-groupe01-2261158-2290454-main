#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "../utils/timer.c"

typedef struct {
    int critical;
    int error;
    int failed_login;
    pid_t pid;
} Result;

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
    return 0;
}

void analyze_block(const char *filename, off_t start, off_t end, int write_fd, int writeZero) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) exit(1);

    char buffer[end - start];
    Result result = {0, 0, 0, getpid()};
    if (writeZero != 1){
        off_t bytes_read = 0;
        while (1 == 1) {
            lseek(fd, start + bytes_read, SEEK_SET);
            ssize_t r = read(fd, buffer, end - start - bytes_read);
            if (r <= 0) break;

            buffer[r] = '\0';
            char* foundPos = strstr(buffer, "CRITICAL");
            if (foundPos) {
                int offset = foundPos - buffer;
                result.critical++;
                bytes_read += offset + strlen("CRITICAL");
            } else {
                break;  
            } 
        }
        bytes_read = 0;
        while (1 == 1) {

            lseek(fd, start + bytes_read, SEEK_SET);
            ssize_t r = read(fd, buffer, end - start - bytes_read);
            if (r <= 0) break;

            buffer[r] = '\0';
            char* foundPos = strstr(buffer, "ERROR");
            if (foundPos) {
                int offset = foundPos - buffer;
                result.error++;
                bytes_read += offset + strlen("ERROR");
            } else {
                break;  
            } 
        }
        bytes_read = 0;
        while (1 == 1) {

            lseek(fd, start + bytes_read, SEEK_SET);
            ssize_t r = read(fd, buffer, end - start - bytes_read);
            if (r <= 0) break;

            buffer[r] = '\0';
            char* foundPos = strstr(buffer, "FAILED_LOGIN");
            if (foundPos) {
                int offset = foundPos - buffer;
                result.failed_login++;
                bytes_read += offset + strlen("FAILED_LOGIN");
            } else {
                break;  
            } 
        }
    }
    write(write_fd, &result, sizeof(Result));
    close(fd);
    close(write_fd);
    exit(0);
}

void process_file(const char *filename, int n, int write_fd) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) exit(1);

    off_t file_size = lseek(fd, 0, SEEK_END);
    off_t block_size = file_size / n;
    close(fd);

    int pipe_fds[n][2];
    pid_t pids[n];

    int lineEnd = 0;
    int lineStart = 0;
    off_t lastBlock = 0;
    for (int i = 0; i < n; i++) {
        pipe(pipe_fds[i]);
        off_t blockEnd = (i == n - 1) ? file_size : lastBlock + block_size;
        lineStart = lineEnd;
        lineEnd = findLineEnd(blockEnd, filename);
        if (lineEnd > lineStart) {
            if ((pids[i] = fork()) == 0) {
                close(pipe_fds[i][0]);
                analyze_block(filename, lineStart, lineEnd, pipe_fds[i][1], 0);
            }
        } else {
            if ((pids[i] = fork()) == 0) {
                close(pipe_fds[i][0]);
                analyze_block(filename, lineStart, lineEnd, pipe_fds[i][1], 1);
            }
        }
        lastBlock = blockEnd;
        close(pipe_fds[i][1]);
    }

    Result total = {0, 0, 0, getpid()};
    Result child_result;
    for (int i = 0; i < n; i++) {
        read(pipe_fds[i][0], &child_result, sizeof(Result));
        total.critical += child_result.critical;
        total.error += child_result.error;
        total.failed_login += child_result.failed_login;
        printf("Child PID de bloc: %d\n", child_result.pid);
        close(pipe_fds[i][0]);
        waitpid(pids[i], NULL, 0);
    }

    write(write_fd, &total, sizeof(Result));
    exit(0);
}

void section2(void *arg) {

    int n = *(int *)arg;

    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe1[0]);
        process_file("logs.txt", n, pipe1[1]);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe2[0]);
        process_file("logs_2.txt", n, pipe2[1]);
    }

    close(pipe1[1]);
    close(pipe2[1]);

    Result res1, res2;
    read(pipe1[0], &res1, sizeof(Result));
    read(pipe2[0], &res2, sizeof(Result));
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    FILE *out = fopen("RESULT_PROCESS.txt", "w");
    fprintf(out, "logs.txt: CRITICAL=%d ERROR=%d FAILED LOGIN=%d\n", res1.critical, res1.error, res1.failed_login);
    fprintf(out, "logs_2.txt: CRITICAL=%d ERROR=%d FAILED LOGIN=%d\n", res2.critical, res2.error, res2.failed_login);
    fclose(out);
    
    printf("Child PID de fichier: %d\n", res1.pid);
    printf("Child PID de fichier: %d\n", res2.pid);

    printf("Parent PID: %d\n", getpid());
}

int main() {


    int n;
    printf("Entrez le nombre de blocs : ");
    scanf("%d", &n);

    double time = measure_time(section2, &n);
    printf("Temps d'exécution avec processus : %.6f secondes\n", time);
}
