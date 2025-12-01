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
         perror("fopen fail");
         strncpy(content, "9999", max_len-1);
         content[max_len-1] = '\0';
         return;
     }
     else{
         fseek(file, 0, SEEK_END); //  want to check file size
         long size = ftell(file);
         fseek(file, 0, SEEK_SET); // reset file pointer back to beginning
 
         if (size > 0) {
             if ((size_t)size >= max_len) {
                 fread(content, 1, max_len-1, file);
                 content[max_len-1] = '\0';
             } else {
                 fread(content, 1, (size_t)size, file);
                 content[size] = '\0'; // null terminate 
             }
             size_t len = strlen(content);
             while (len > 0 && (content[len-1] == '\n' || content[len-1] == '\r')) {
                 content[len-1] = '\0';
                 len--;
             }
         } else {
             content[0] = '\0';
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
             size_t len = strlen(rubric[i]);
             if (len > 0 && (rubric[i][len-1] == '\n' || rubric[i][len-1] == '\r')) {
                 rubric[i][len-1] = '\0';
             }
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
         if (rubric[i][0] != '\0') {
             fputs(rubric[i], file);
         }
         fputc('\n', file); // new line because of formatting
      }
      fclose(file);
 }
 
 void ta_delay(double min, double max){ // handling the random delay for TA marking
     double range = max - min;
     double delay = 0.0;
     if (RAND_MAX > 0) delay = (double) rand() / (double) RAND_MAX;
     double sleep = min + delay * range;
     usleep((useconds_t)(sleep * 1000000)); // convert to microseconds
 }
 
 int main(int argc, char* argv[]){
     if (argc < 2) {
         printf("Usage: %s <num_of_TAs>\n", argv[0]);
         exit(1);
     }
     int ta_count = atoi(argv[1]); // number of TAs from command line argument
 
     if (ta_count < 2) {  
         printf("ERROR: need at least 2 TAs\n");
         return 1;
     }
 
     /* Create shared memory */
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
             printf("(TA %d) : Marking student %s (pid=%d)\n", i, shared->student_id, (int)getpid());
 
             while(!shared->completed){
                 // if student id "9999" is encountered, mark complete
                 if (strncmp(shared->student_id, "9999", 4) == 0) {
                     shared->completed = 1;
                     printf("(TA %d) : Found student 9999. Setting completed=1\n", i);
                     break;
                 }
 
                 printf("(TA %d) : Reviewing rubric for student %s\n", i, shared->student_id);
 
                 //review rubric (may modify it) 
                 for (int q = 0; q < 5; q++) {
                     printf("(TA %d) : Reviewing line %d of rubric: %s\n", i, q+1, shared->rubric[q]);
                     ta_delay(0.5, 1.0); // simulate the delay for reviewing rubric
 
                     if((rand() % 5) == 0){ // 20% chance that the rubric needs modifying
                         printf("(TA %d) : Identified a mistake in rubric line %d. Modifying...\n", i, q + 1);
 
                         // find a comma then increment character after it
                         char* comma = strchr(shared->rubric[q], ',');
                         if (comma != NULL && *(comma + 1) != '\0') {
                             char* rubric_char = comma + 1;
                             /* skip possible space */
                             if (*rubric_char == ' ') rubric_char++;
                             if (*rubric_char != '\0') {
                                 (*rubric_char)++;
                                 printf("(TA %d) : Modified rubric line %d to: %s\n", i, q + 1, shared->rubric[q]);
                                 write_rubric("rubric.txt", shared->rubric); // write back to rubric file, save
                             }
                         }
                     }
                 }
 
                 // mark qs or load next exam
                 if(shared->qs_done < 5){ // check if there are questions left to mark
                     int current_question = shared->qs_done + 1;
                     printf("(TA %d) : Marking question %d for student %s\n", i, current_question, shared->student_id);
                     ta_delay(1.0, 2.0); // simulate delay for marking question
                     
                     shared->qs_done++;
                     printf("[TA %d] Finished marking question %d for Student %s\n", i, current_question, shared->student_id);
                 }
                 else{
                     // next exam loading
                     shared->curr_exam++;
                     shared->qs_done = 0;
 
                     char next_file[50];
                     if (shared->curr_exam <= 20) {
                         snprintf(next_file, sizeof(next_file), "exam%02d.txt", shared->curr_exam);
                     } else {
                         snprintf(next_file, sizeof(next_file), "exam20.txt");
                     }
                     printf("(TA %d) : All questions marked for previous student. Loading: %s\n", i, next_file);
                     load_exam_file(next_file, shared->student_id, sizeof(shared->student_id));
 
                     if (strncmp(shared->student_id, "9999", 4) == 0) {
                         printf("(TA %d) : No more exams to mark!\n", i);
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
 
     //cleaning up
     shmdt(shared);
     shmctl(shmid, IPC_RMID, NULL);
     printf("\nShared memory cleaned up.\n");
 
     return 0;
 }
 