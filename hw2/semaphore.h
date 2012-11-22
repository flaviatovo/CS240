//
// File created by Flavia Tovo as work for project 2
// of OS class on KAUST
// Fall 2012
//

// CONSTANTS
#define NSEM 10 // Maximum number of semaphores on system

// RETURN CODES
#define SEM_OK 0
#define SEM_DOES_NOT_EXIST -1
#define OUT_OF_SEM -2

// This struct is used to store information about 1 semaphore,
// the list of all semaphores should be stored in a different place
struct semaphore {
  int value;         // stores the value of the semaphore
  uint name;         // stores the name of the semaphore, given in sem_get
};

// Function used to get or create a semaphore
// If a semaphore with name equal to name exists, than return 
// the handler of that semaphore;
// If a semaphore with name doesn't exist, create the semaphore,
// initialize the value as value and return the handler.
// int sem_get(uint name, int value);

// Function used to delete a semaphore, if there are processes waiting
// on that semaphore, they can't be let spleeping forever!
// int sem_delete(int handle);

// Used to increase the value of the semaphore, if the value was previously
// 0, this should wake processes sleeping on name
// int sem_signal(int handle);

// Used to wait for an event on the semaphore, if the value is greater than 0
// should just decrement the value and continue working
// if value is 0, sleep on name
// int sem_wait(int handle);