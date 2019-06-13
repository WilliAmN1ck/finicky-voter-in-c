# CS441/541 Project 5

## Author(s): Nick Williams

TODO


## Date: 3/27/18

TODO


## Description:
This program simulates voting at a polling station with 3 voting parties and given input of number of booths and number of voters in each party.

TODO


## How to build the software
NOTE: finicky-voter2.c is a duplicate program that I used while creating the program, finicky-voter.c is the program to run

compile the program in the terminal or type make (while in the correct directory) in the commandline to compile the program.
TODO


## How to use the software
Start by typing ./finicky-voter b r d i where b is the number of booths, r is the number of republicans, d is the number of democrats and i is the number of independents voting.
The program must take in arguments > 0 for each of these inputs or else the program will use default values of 10, 5, 5, 5 respectively
TODO


## How the software was tested
The software was tested using different numbers for each variable. Each variable was tested with numbers > 10 and < 10.
The number of booths was also changed for each test
tests included:
    number of booths < 5: parties each tested with the same # (> 10 and < 10) as well as one party being >10 while the rest are < 10
        these tests were duplicated at least 5 times per test in order to ensure no deadlock
    number of booths > 5: parties each tested with the same # (> 10 and < 10) as well as one party being >10 while the rest are < 10
        these tests were duplicated at least 5 times per test in order to ensure no deadlock (if deadlock was detected during a test,
        then changes to the code were made until deadlock did not occur in testing at least 10-15 times)
    In addition to ensuring no deadlock, the order of the print satements was evaluated on most of the tests in order to ensure that
    proper ordering was being followed with entering the polling station and voting per each voter and parties
    The status of each booth and when it was beign updated and printed out was evaluated in order to ensure proper ordering
TODO


## Known bugs and problem areas
1. If an independent enters first,  & then another party after to take control of the cur_voting_party variable,
it had issues with deadlock for a while -- BUT I think I fixed it with adding a semaphore_post dem and rep parties
to signal any of the same party waiting for the mutex -- sometimes got stuck waiting to be signaled while the party was supposed
to be voting (if else clause before while loop within each party method)
2. Get an error when trying to destroy semaphores in the queue at position i, so just destroyed the whole queue at the end
3. Sometimes with a party size of 1 or small & others with larger parties... the party of 1 does not starve but if it enters toward
    the middle of the program, it will more than likely vote more toward the end if not one of the last
TODO

## Special Section
1&2. Firstly, I used a barrier in my solution in order to start each thread at which point they waited until the rest of the
threads finally entered. As the threads enter their respective methods, a semaphore within their thread is created and
added to a queue of semaphores in order to preserve the ordering in which they came.
After this point, the main thread sleeps for 2 seconds and after this I signal my barrier semaphore
which signals the rest of the threads that they can line up to vote. Throughout the code, I am also using a mutex semaphore,
whose basic function is to just protect variables and print statements in order to preserve their ordering and make sure
that only one thread has access to it at a time. I simulated the sign in time randomly in my Sign_in_Desk
method and then after they sign in, they wait to be signaled by the main thread that they can go in. Once they sign in, I check
to see if their party is currently voting or not, if it is not then they must wait until they are signaled to do so. I implemented
this with two semaphores, a dem_mutex and a rep_mutex initialized to 0. Once they are able to vote, the threads must
check to see if there are any of the opposite party still voting. Once there are none of the opposite party voting,
then they can proceed to check booths. I used a semaphore pointer  to point to an array of semaphores initialized to 1
called booths_mutex. This is used to make sure that threads are not trying to vote in the same booth as well as protecting
my char pointer variable called string_booth_status as that variable is keeping track of where a voter is voting for
printing to standard out. Once the voter votes, I make sure to decrement book keeping variables while in control of the
semaphore mutex. Then once the voter is done, I have some checks to see if I should keep having the same party vote
or if it should change to the other party. I let independents vote wherever they are in the order of the queue because
I felt like that was most fair, if they are not causing any issues with democrats or republicans then this should make it go efficiently.
I felt that the most fair way of checking if the opposite party should take over voting would be if there is (# booths/2)x the number
of the opposite party waiting than the number of the party currently voting waiting to vote yet.

2. My solution avoids deadlock by using a counting semaphore in the main thread to allow, at max the number of booths of threads,
having access to the voting section of the code (threads may "enter" the polling station before they are signaled in my solution). As
stated above, I used a semaphore named mutex to only allow access to variables and print statements throughout the code. I also
implemented  barrier for the threads as they come in to the methods. I was also to make sure that I only allowed one type of party
out of democrats and republicans in to vote at a time through two semaphores named dem_mutex and rep_mutex. Once a voter
is done voting, it checks to see if it should signal a democrat to vote or a republican. I also have checks before the voting section
for threads to see if they should let the opposite party go, or signal another of the same party to enter if one is waiting.
I defined fairness as threads should be signaled as they come in, but if one party has a longer line of waiting people to vote they will
voting priority, unless the party with the shorter line was due to it having a much smaller party size to vote. Starvation was also
prevented in this way through pre-empting threads with small parties to vote if they were waiting for a while. My definition of fairness
was intended to be that as the voters were signaled from the main thread, they should be able to vote as long as their line is not (#
booths/2)x longer than the other non-voting party. I also extended fairness to include that if a party had a much smaller initial
size than the voting party, that the voter of the smaller size should be able to vote as it is called. Basically, fairness was determined
by the party size and how many voters of each party were left waiting to vote with proportion to their original part size.
However, if one or a small # of voters from a party come in late, they should vote as they came in while taking in the condtionals stated
above. This may result in a little bit of a lag as they have to let the other party stop voting or ones that were waiting on booths to vote.
However, it should solve starvation to a degree for the most part. I also included what I think would act as pre-emption clauses
at the end of independent voters in order to get other parties to start voting if they were ever stuck waiting or if they were not
ever signaled due to an independent arriving and voting first.
