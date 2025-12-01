/**
 * @file part2a_marking_student1_studen2.cpp
 * @author Akshavi Baskaran
 * 
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

typedef struct{
    char    rubric[5][256]; // 5 lines in the rubric, each line max 256 chars
    char    student_id[32]; // student number 
    int     curr_exam; // index of the current exam being marked
    int     qs_done; // number of questions done
    int     completed; // flag for marking done or not
} shared_data;

void load_exam_file(char* name, char* content){ // method for loading the exam files, reading them
    FILE* file = fopen(name, "r"); // reading file
    if (file == NULL){
        perror("fopen fail");
        exit(1);
    }
    else{
        fseek(file, 0, SEEK_END); //  want to check file size
        long size = ftell(file);
        fseek(file, 0, SEEK_SET); // reset file pointer back o beginning

        if (size > 0){
            fread(content, size, 1, file); // read the entire file into content buffer
        }
        content[size] = '\0'; // null terminate 
        fclose(file);
    }
}

void load_rubric_file(char* name, char rubric[5][256]){ // method for loading the single rubric file, reading it
    FILE* file = fopen(name, "r"); 
    if (file == NULL){
        perror("fopen fail");
        exit(1);
    }
     for (int i = 0; i < 5; i++) { // read 5 rubric lines
        fgets(rubric[i], 256, file);
     }
     fclose(file);
}

void write_rubric(char* name, char rubric[5][256]){ // method for writing the single rubric file, writing in it
    FILE* file = fopen(name, "w"); 
    if (file == NULL){
        perror("fopen fail");
        exit(1);
    }
     for (int i = 0; i < 5; i++) { // write 5 rubric lines
        fputs(rubric[i], file);
        fputs("\n", file); // new line because of formatting
     }
     fclose(file);
}

void ta_delay(double min, double max){ // handling the random delay for TA marking
    double range = max - min;
    double delay = (double) rand() / (double) RAND_MAX;
    double sleep = min + delay * range;
    usleep((useconds_t)(sleep * 1000000)); // convert to microseconds
}

int main(int argc, char* argv[]){
    int ta_count = argv[1]; // number of TAs
    if (ta_count < 2) {  
        printf("ERROR: need at least 2 TAs\n");
        return 1;
    }

    int shmid = shmget(IPC_PRIVATE, sizeof(shared_data), IPC_CREAT | 0666);
    if (shmid < 0){
        perror("shmget fail");
        exit(1);
    }
    else{
        shared_data* shared = (shared_data*)shmat(shmid, NULL, 0);
        if (shared == (shared_data*)-1) {
            perror("shmat failed");
            exit(1);
        }
        load_rubric_file("rubric.txt", shared->rubric); // load rubric file into shared memory
        load_exam_file("student1.txt", shared->student_id); // load student1 exam file into shared memory

        /**printf("\nTTTESESSSTINGGGG\n"); // test 
        printf("studen id : %s\n", shared->student_id); //test
        
        printf("rubric in memory:\n"); // test
        for(int i=0; i<5; i++) { //test
            printf("  Line %d: %s", i+1, shared->rubric[i]); //test
        } //test**/

        for(int i=0; i<ta_count; i++){
            pid_t pid = fork();
            if (pid < 0){
                perror("fork fail");
                exit(1);
            }
            else if (pid == 0){
                srand(time(NULL) + getpid()); // seed random number generator
                print("\nTA %d marking student %s\n", i, getpid());

            }
        }

        shmdt(shared);
        shmctl(shmid, IPC_RMID, NULL);
        printf("\nShared memory cleaned up.\n");
    }
}