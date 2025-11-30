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
} shared_data;

void load_exam_file(char* name, char* content){
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

void load_rubric_file(char* name, char rubric[5][256]){
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

int main(){
    int shmid = shmget(IPC_PRIVATE, sizeof(shared_data), IPC_CREAT | 0666);
    if (shmid < 0){
        perror("shmget fail");
        exit(1);
    }
    else{
        //strcpy(shared_data->student_id, "student1");
    }

}