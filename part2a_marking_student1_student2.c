/**
 * @file part2a_marking_student1_student2.cpp
 * @author akshavi baskaran
 * @author liam addie
 * */

 #include <sys/types.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <sys/mman.h> 
 #include <stdlib.h>
 #include <fcntl.h>
 #include <sys/shm.h>
 #include <sys/sem.h>
 #include <sys/stat.h>
 #include <time.h>
 #include <string.h>
 #include <sys/wait.h>   
 #include <errno.h>
 
 typedef struct{
     char    rubric[5][256]; // 5 lines in the rubric, each line max 256 chars
     char    student_id[32]; // student number 
     int     curr_exam; // index of the current exam being marked
     int     qs_done; // number of questions done
     int     completed; // flag for marking done or not
 } shared_data;
 
 void load_exam_file(const char* name, char* content, size_t max_len){ // method for loading the exam files, reading them
     FILE* file = fopen(name, "r"); // reading file
     if (file == NULL){
         // if file fails to open (race condition or end of files), signal stop
         strncpy(content, "9999", max_len-1);
         content[max_len-1] = '\0';
         return;
     }
     else{
         if (fgets(content, max_len, file) == NULL) {
             strncpy(content, "9999", max_len-1);
             content[max_len-1] = '\0';
         } else {
             // remove newline characters
             content[strcspn(content, "\n\r")] = '\0';
         }
         fclose(file);
     }
 }
 
 void load_rubric_file(const char* name, char rubric[5][256]){ // method for loading the single rubric file, reading it
     FILE* file = fopen(name, "r"); 
     if (file == NULL){
         perror("fopen fail");
         // initialize rubric lines to empty to avoid uninitialised data 
         for (int i = 0; i < 5; ++i) rubric[i][0] = '\0';
         return;
     }
     for (int i = 0; i < 5; i++) { // read up to 5 rubric lines
         if (fgets(rubric[i], 256, file) == NULL) {
             rubric[i][0] = '\0';
         } else {
             // remove newline characters
             rubric[i][strcspn(rubric[i], "\n\r")] = '\0';
         }
      }
      fclose(file);
 }
 
 void write_rubric(const char* name, char rubric[5][256]){ // method for writing the single rubric file, writing in it
     FILE* file = fopen(name, "w"); 
     if (file == NULL){
         perror("fopen fail");
         return;
     }
      for (int i = 0; i < 5; i++) { // write 5 rubric lines
         fprintf(file, "%s\n", rubric[i]); // write line with newline
      }
      fclose(file);
 }
 
 void ta_delay(double min, double max){ // handling the random delay for ta marking
     double range = max - min;
     double delay = (double)rand() / RAND_MAX;
     double sleep = min + delay * range;
     usleep((useconds_t)(sleep * 1000000)); // convert to microseconds
 }
 
 int main(int argc, char* argv[]){
     if (argc < 2) {
         printf("Usage: %s <num_of_TAs>\n", argv[0]);
         exit(1);
     }
     int ta_count = atoi(argv[1]); // number of tas from command line argument
 
     if (ta_count < 2) {  
         printf("ERROR: need at least 2 TAs\n");
         return 1;
     }
 
     // create shared memory 
     int shmid = shmget(IPC_PRIVATE, sizeof(shared_data), IPC_CREAT | 0666);
     if (shmid < 0){
         perror("shmget fail");
         exit(1);
     }
 
     shared_data* shared = (shared_data*)shmat(shmid, NULL, 0);
     if (shared == (shared_data*)-1) {
         perror("shmat failed");
         shmctl(shmid, IPC_RMID, NULL);
         exit(1);
     }
 
     // initialize shared memory
     load_rubric_file("rubric.txt", shared->rubric); // load rubric file into shared memory
     load_exam_file("exam01.txt", shared->student_id, sizeof(shared->student_id)); // load first exam file into shared memory
 
     shared->curr_exam = 1; // start with first exam
     shared->qs_done = 0; // no questions marked yet
     shared->completed = 0; // marking not completed
 
     printf("Master: Starting with %d TAs and student: %s\n", ta_count, shared->student_id);
 
     // fork ta process 
     for(int i=0; i<ta_count; i++){
         pid_t pid = fork();
         if (pid < 0){
             perror("fork fail");
             shmdt(shared);
             shmctl(shmid, IPC_RMID, NULL);
             exit(1);
         }
         else if (pid == 0){
             // child process
             srand((unsigned)(time(NULL) ^ getpid())); // seed random number generator
             printf("(TA %d) : Started (pid=%d)\n", i, (int)getpid());
 
             while(!shared->completed){
                 // if student id "9999" is encountered, mark complete
                 if (strncmp(shared->student_id, "9999", 4) == 0) {
                     shared->completed = 1;
                     printf("(TA %d) : Found student 9999.\n", i);
                     break;
                 }
 
                 printf("[REVIEW] (TA %d) : Reviewing rubric for student %s\n", i, shared->student_id);
 
                 // review rubric (may modify it) 
                 for (int q = 0; q < 5; q++) {
                     ta_delay(0.5, 1.0); // simulate the delay for reviewing rubric
 
                     if((rand() % 10) == 0){ // 10% chance that the rubric needs modifying
                         printf("[REVIEW] (TA %d) : Identified a mistake in rubric line %d. Modifying...\n", i, q + 1);
 
                         // find a comma then increment character after it
                         char* comma = strchr(shared->rubric[q], ',');
                         if (comma != NULL && *(comma + 2) != '\0') {
                             char* rubric_char = comma + 2;
                             (*rubric_char)++;
                             printf("[REVIEW] (TA %d) : Modified rubric line %d to: %s\n", i, q + 1, shared->rubric[q]);
                             write_rubric("rubric.txt", shared->rubric); // write back to rubric file, save
                         }
                     } else {
                         // logging update: explicitly state no changes were made
                         printf("[REVIEW] (TA %d) : Line %d , No changes made.\n", i, q + 1);
                     }
                 }
                 if (shared->completed || strncmp(shared->student_id, "9999", 4) == 0) {
                    shared->completed = 1; // ensure flag is set
                    break; // break the while loop
                }
 
                 // mark qs or load next exam
                 if(shared->qs_done < 5){ // check if there are questions left to mark
                     int current_question = shared->qs_done + 1;
                     
                     // logging update: print before entering critical section (marking)
                     printf("[MARK] (TA %d) : Attempting to mark question %d for student %s\n", i, current_question, shared->student_id);
                     
                     ta_delay(1.0, 2.0); // simulate delay for marking question
                     
                     // logging update: check if race condition occurred during delay
                     if (shared->qs_done >= 5) {
                         printf("[MARK] (TA %d) : Question %d was already marked by another TA while I was thinking.\n", i, current_question);
                     } else {
                         shared->qs_done++;
                         printf("[MARK] (TA %d) : Successfully finished marking question %d for Student %s\n", i, current_question, shared->student_id);
                     }
                 }
                 else{
                     // next exam loading
                     shared->curr_exam++;
                     shared->qs_done = 0;
 
                     char next_file[50];
                     if (shared->curr_exam <= 20) {
                         snprintf(next_file, sizeof(next_file), "exam%02d.txt", shared->curr_exam);
                     } else {
                         // after exam20, stop trying to load more files
                         strcpy(shared->student_id, "9999");
                         printf("[LOAD] (TA %d) : No more exam files. Setting student_id to 9999\n", i);
                         continue;
                     }
                     printf("[LOAD] (TA %d) : All questions marked for previous student. Loading: %s\n", i, next_file);
                     load_exam_file(next_file, shared->student_id, sizeof(shared->student_id));
 
                     if (strcmp(shared->student_id, "9999") == 0) {
                         printf("[LOAD] (TA %d) : No more exams to mark!\n", i);
                     }
                 }
             } // end while
 
             printf("(TA %d) : Exiting marking process.\n", i);
             shmdt(shared);
             exit(0);
         }
         // next fork 
     }
 
     // waiting for the children
     printf("Master: Waiting for %d TAs to finish...\n", ta_count);
     for(int i = 0; i < ta_count; i++) {
         wait(NULL);
     }
 
     // cleaning up
     shmdt(shared);
     shmctl(shmid, IPC_RMID, NULL);
     printf("\nShared memory cleaned up.\n");
 
     return 0;
 }