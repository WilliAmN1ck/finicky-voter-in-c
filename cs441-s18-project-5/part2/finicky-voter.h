/*
 * 
 *
 * CS 441/541: Finicky Voter (Project 5)
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "semaphore_support.h"

/*****************************
 * Defines
 *****************************/

/* 1.0 second = 1000000 usec */
#define VOTE_TIME 100000
/* 0.5 second = 500000 usec */
#define SIGN_IN_TIME 50000

#define TRUE  0
#define FALSE 1

/*****************************
 * Structures
 *****************************/
//structure for each type of voter
struct voter {
    int voter_type; //NOT USED
    int voter_id; //id # for each voter
    int num_times_waited; //NOT USED
    int did_vote;
};
typedef struct voter voter;

struct args {
    int tid;//thread id passed in creation
    int pos; //position of voter in voting array
};
typedef struct args args;

/*struct for storing the voters that are waiting in each booth line*/
/*inner counter variable in this struct for number of voters in line?*/
/*need to put next voter in the shortest/best fit line?*/
/*each booth have its own wait list?*/

/*****************************
 * Global Variables
 *****************************/
/* maybe need these as globals... maybe not*/
int booths_open; // 0 == true; 1 == false; may not need
int num_booths;
int cur_voting_party; //0 = ind, 1 == rep, 2 == dem

int num_republicans;
int num_democrats;
int num_independents;

int reps_waiting;
int dems_waiting;
int inds_waiting;
int total_voters;
int reps_voted;
int dems_voted;
int inds_voted;
int inds_cur_voting;
int reps_cur_voting;
int dems_cur_voting;
char* string_booth_status;

int* booth_status; /*keep track of the status of each booth so know which one is being voted in
                    0 = open, 1 = false*/
int num_voted; //number of people that have voted

voter* rep_voters;
voter* dem_voters;
voter* ind_voters;

/*semaphore declarations*/
semaphore_t* booths_mutex; /* NOT USED*/
/*use an integer array of true and falses instead...?*/
semaphore_t mutex; /*guard against a certain variable manipulation..?*/
semaphore_t party_mutex; /*NOT USED*/

semaphore_t barrier;
semaphore_t** queue; //an array to act as a FIFO Q
semaphore_t self_sem;
semaphore_t voter_sem;/*rendezvous counting semaphores so less voters trying to access crit section*/
semaphore_t voter_done;
semaphore_t poll_station_done;

semaphore_t rep_mutex;/*instead of a party mutex have these initialized to 0*/
semaphore_t dem_mutex;
semaphore_t ind_mutex;
/*could use a queue of waiting semaphores to preserve order?*/

/*****************************
 * Function Declarations
 *****************************/
/*function for threads simulating republican voters*/
void* Republican(void* thread_id);
/*function for threads simulating democratic voters*/
void* Democrat(void* thread_id);
/*function for threads simulating independent voters*/
void* Independent(void* thread_id);
/*function for threads to simulate sign-in time - sleep in this thread
 * int voter_pos is not used/needed
 */
void Sign_In_Desk(int voter_pos);
/*function for threads to simulate time to thoughtfully vote- sleep here
 * int voter_pos not needed
 * voter* cur_voter is the voter that is simulating the vote
 * set did_vote for this voter to true.
 */
void vote_sim(voter* cur_voter, int voter_pos);
