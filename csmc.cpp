// Compile: g++ -pthread csmc.cpp -o csmc
// Run: ./csmc 10 3 4 5

#include <pthread.h>
#include <stdio.h>
#include <iostream> 
#include <stdlib.h>  
#include <random>
#include <chrono>
#include <unistd.h>

using namespace std;  

const int MINUTE = 1000 * 1000;

struct Options { // Can only pass one variable to cordinator thread
  int students;
  int tutors;
  int chairs;
  int help;  
};

void *cordinate(void *thread_args)
{
  struct Options *args;
  args = (struct Options *) thread_args;
  cout << args->help;
  
  cout << "Cordinator Started \n";
  usleep(5 * MINUTE);
  
  // Create a thread for each student
  
  // Create a thread for each tutor
  
  // Wait for all tutor threads to return
  
}

int main(int argc, char *argv[])
{
  Options *args, o;
  args = &o;
  args->students = atoi(argv[1]); // Storing arguments as ints
  args->tutors = atoi(argv[2]);
  args->chairs = atoi(argv[3]);
  args->help = atoi(argv[4]);  
  
  // Create a cordinator thread
  pthread_t cordinator;
  
  pthread_create(&cordinator, NULL, cordinate, (void *)&args[0]); // Jumps to Cordinate

  // Waiting for the cordinator thread to finish (means all tutor threads have finished)
  pthread_join(cordinator, NULL);
  cout << "Cordinator Finished \n";

}