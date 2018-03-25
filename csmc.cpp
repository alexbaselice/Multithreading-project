// Compile: g++ -std=c++11 -pthread csmc.cpp -o csmc
// Run: ./csmc 10 3 4 5

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <random>
#include <chrono>
#include <unistd.h>
#include <map>
#include <semaphore.h>

using namespace std;

const int SECOND = 1000 * 1000;
enum class State { RUNNING, FINISHED };

struct Options { // Can only pass one variable to cordinator thread
	int students;
	int tutors;
	int chairs;
	int help;
};

map<pthread_t, State> students;
map<pthread_t, State> tutors;

map<pthread_t, int> chairs;
map<pthread_t, pthread_t> tutor_to_student;

sem_t chairs_sem;
sem_t tutor_to_student_sem;

int chairs_filled(vector<int> chairs)
{
	int chairs_filled = 0;
	for (auto &chair : chairs)
	{
		if (chair != 0) chairs_filled++;
	}
	return chairs_filled;
}

void *be_student(void *thread_args)
{
	int *help_ref = (int *)thread_args;
	int help = int(*help_ref);
	pthread_t id = pthread_self();
	printf("Student started id = %d\n", id);

	// Check if there's an empty seat
	bool found_seat = false;

	// While Student still hasn't finished counting down help number
	while (help > 0)
	{
		if (!found_seat)
		{
			//wait for the chairs array to be free
			sem_wait(&chairs_sem);

			if (chairs.size() < 4)
			{
				found_seat = true;
				chairs[id] = 1;
				printf("Student [%d] takes a seat. Waiting students = [%d]\n", id, chairs.size());
			}

			sem_post(&chairs_sem);
			if (!found_seat)
			{
				// Else student found no chair and will come back later
				printf("Student [%d] found no empty chair will try again later\n", id);
				usleep(1 * SECOND);
			}
		}

		if (found_seat)
		{
			// Wait for tutors to be avaible

			sem_wait(&tutor_to_student_sem);

			// If student is being helped, decrement help
			for (auto const &pair : tutor_to_student) // For all waiting students
			{
				if (pair.second == id) { help--; }
			}

			// Taken a tutor
			sem_post(&tutor_to_student_sem);

			// Programing for a random ammount of time
			usleep(1 * SECOND);
		}
	}

	// Remove the students chair, student has left
	sem_wait(&chairs_sem);
	chairs.erase(id);
	sem_post(&chairs_sem);

	// Set thread to finished
	printf("Student finished id = %d\n", id);
	students[id] = State::FINISHED;

}

void *be_tutor(void *thread_args)
{
	pthread_t id = pthread_self();
	printf("Tutor started id = %d\n", id);

	// While there are people waiting
	bool people_waiting = false;
	bool currently_tutoring = false;
	sem_wait(&chairs_sem);
	if (chairs.size() > 0) people_waiting = true;
	sem_post(&chairs_sem);
	while (people_waiting)
	{

		// If there is a waiting student and waiting tutor pair them up
		sem_wait(&chairs_sem);
		sem_wait(&tutor_to_student_sem);
		if (tutor_to_student.find(id) != tutor_to_student.end()) // Currently helping a student
		{
			currently_tutoring = true;
			printf("%d is current tutoring\n", id);
		}
		if (!currently_tutoring)
		{
			for (auto const &chair : chairs) // For all waiting students
			{
				if (tutor_to_student.find(id) != chair.first) // If not already helping this student
				{
					tutor_to_student[id] = chair.first; // Assign this tutor to the student
					printf("Tutor [%d] helping student for [%d] seconds. Waiting students = [%d]\n", id, chair.first, chairs.size());
					break;
				}
			}
		}
		sem_post(&tutor_to_student_sem);
		sem_post(&chairs_sem);

		// Tutoring
		if (currently_tutoring)
		{
			usleep(1 * SECOND);

			// Finished Tutoring
			currently_tutoring = false;
			sem_wait(&tutor_to_student_sem);
			tutor_to_student.erase(id);
			sem_post(&tutor_to_student_sem);
		}


	}
	// Wait for a student to be assigned


	// Set thread to finished
	printf("Tutor finished id = %d\n", id);
	tutors[id] = State::FINISHED;

}


void *coordinate(void *thread_args)
{
	struct Options *args;
	args = (struct Options *) thread_args;
	cout << "Coordinator Started \n";

	// Creating semaphores
	sem_init(&chairs_sem, 0, 1);
	sem_init(&tutor_to_student_sem, 0, 1);


	// Create a thread for each student
	int help = args->help;
	for (int i = 0; i < args->students; i++)
	{
		pthread_t student;
		students[student] = State::RUNNING;
		pthread_create(&student, NULL, be_student, &help);
	}

	// Create a thread for each tutor
	for (int i = 0; i < args->tutors; i++)
	{
		pthread_t tutor;
		tutors[tutor] = State::RUNNING;
		pthread_create(&tutor, NULL, be_tutor, NULL);
	}

	// Wait for all tutor threads and student threads to set state to FINISHED
	bool students_finished = false;
	bool tutors_finished = false;
	while (!students_finished || !tutors_finished)
	{
		// Check if everyone is finished
		for (auto const &student : students)
		{
			if (student.second == State::FINISHED)
			{
				students_finished = true;
			}
			else { students_finished = false; }
		}
		for (auto const &tutor : tutors)
		{
			if (tutor.second == State::FINISHED)
			{
				tutors_finished = true;
			}
			else { tutors_finished = false; }
		}
	}
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
	pthread_t coordinator;
	pthread_create(&coordinator, NULL, coordinate, (void *)&args[0]); // Jumps to Cordinate

																	  // Waiting for the cordinator thread to finish (means all tutor threads have finished)
	pthread_join(coordinator, NULL);
	cout << "Cordinator Finished \n";

}