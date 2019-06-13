/*
 *
 *
 * CS 441/541: Finicky Voter (Project 5)
 *
 */
#include "finicky-voter.h"

int main(int argc, char * argv[]) {
    booths_open = 1; //set booths_open to false;
    /*initialize possible cmdline inputs to default values*/
    num_booths = 10;
    num_republicans = 5;
    num_democrats = 5;
    num_independents = 5;
    num_voted =0; //number of voters to vote
    reps_waiting = 0;
    dems_waiting = 0;
    inds_waiting = 0;
    inds_voted = 0;
    reps_voted= 0;
    dems_voted = 0;
    inds_cur_voting = 0;
    reps_cur_voting =0;
    dems_cur_voting =0;
    cur_voting_party = -1; //initialize cur_voting_party to -1 so no party gets mutex right away
    
    if (argc > 1 && atoi(argv[1]) > 0) {
        num_booths = atoi(argv[1]);
    }
    if (argc > 1 && atoi(argv[2]) > 0) {
        num_republicans = atoi(argv[2]);
    }
    if (argc > 2 && atoi(argv[3]) > 0) {
        num_democrats = atoi(argv[3]);
    }
    if (argc > 3 && atoi(argv[4]) > 0) {
        num_independents = atoi(argv[4]);
    }
    
    booth_status = (int*)malloc(sizeof(int) *num_booths);
    string_booth_status = (char*)malloc(sizeof(char)*(num_booths*3));
    /*seed random number generator for thread sleeping time*/
    srandom(time(NULL));
    
    booths_mutex = (semaphore_t*)malloc(sizeof(semaphore_t)*num_booths);
    /*initialize semaphores to their respective counts & the booth status'*/
    int i = 0;
    for(i = 0; i < num_booths; i++) {
        //mutex for each booth to tell if open or not
        semaphore_create(&booths_mutex[i],1); //initialize booth mutex to 1 == open
        booth_status[i] = 1; /*intiialize booth_status' to false(1)*/
        strcat(string_booth_status, "[.]");
    }
    
    semaphore_create(&voter_sem, num_booths); //counting semaphore for num_voters allowed in
    semaphore_create(&voter_done, num_booths);
    semaphore_create(&poll_station_done, num_booths);
    
    semaphore_create(&barrier, 0);
    semaphore_create(&party_mutex, 1);
    semaphore_create(&mutex, 1);
    
    semaphore_create(&rep_mutex,0);
    semaphore_create(&dem_mutex,0);
    semaphore_create(&ind_mutex,0); //may not need this.. independents vote for themselvse
    
    printf("Number of Voting booths   : %3d\n", num_booths);
    printf("Number of Republican      : %3d\n", num_republicans);
    printf("Number of Democrat        : %3d\n", num_democrats);
    printf("Number of Independent     : %3d\n", num_independents);
    printf("---------------------+----------------------+--------------------------\n");
    
    int num_total_voters = num_republicans + num_democrats + num_independents;
    total_voters = num_total_voters;
    /*initialize & malloc space for republicans, democrats and independents in voters struct*/

    pthread_t threads[num_total_voters];
    rep_voters = (voter*)malloc(sizeof(voter) * num_republicans);
    dem_voters = (voter*)malloc(sizeof(voter) * num_democrats);
    ind_voters = (voter*)malloc(sizeof(voter) * num_independents);
    
    queue = (semaphore_t**)malloc(sizeof(semaphore_t*)*total_voters);
    
    int rtntid = 0; //variable to store returned thread id from pthread_create
    int rep_pos = 0;
    int dem_pos = 0;
    int ind_pos = 0;
    for(i = 0; i < num_total_voters; i++){
        /* create threads for republicans, democrats and independents*/
        /*intiialize voter type to their voter type & num_times_waited to 0 and did_vote to 1(false)*/
        
        if (i < num_republicans) {
            rep_voters[rep_pos].voter_type = 1;
            rep_voters[rep_pos].voter_id = rep_pos;
            rep_voters[rep_pos].num_times_waited = 0;
            rep_voters[rep_pos].did_vote = 1;
            rtntid = pthread_create(&threads[i], NULL, Republican, (void *)(intptr_t)i);
            rep_pos++;
        } else if ( i < num_republicans + num_democrats){
            dem_voters[dem_pos].voter_type = 2;
            dem_voters[dem_pos].voter_id = dem_pos;
            dem_voters[dem_pos].num_times_waited=0;
            dem_voters[dem_pos].did_vote = 1;
            rtntid = pthread_create(&threads[i], NULL, Democrat, (void *)(intptr_t)i);
            dem_pos++;
        } else {
            ind_voters[ind_pos].voter_type = 0;
            ind_voters[ind_pos].voter_id = ind_pos;
            ind_voters[ind_pos].num_times_waited = 0;
            ind_voters[ind_pos].did_vote = 1;
            rtntid = pthread_create(&threads[i], NULL, Independent, (void *)(intptr_t)i);
            ind_pos++;
        }
        if(rtntid != 0) {
            printf("ERROR: return code from pthread_create is %d\n", rtntid);
        }
      
        
    }
    /*after creating the threads, sleep for two seconds b4 opening poll booths*/
    sleep(2);
    printf("---------------------+----------------------+--------------------------\n");
    semaphore_post(&barrier); //singal the barrier (last thread has arrived)
    booths_open = 0; //booths_open set to true to open booths

    /*loop through the queue and signal semaphores?*/
    
    for(i = 0; i < num_total_voters; i++) {
        semaphore_wait(&voter_sem); //wait until signaled by the voter
        semaphore_wait(&mutex); //should not need this
        semaphore_t sem = *queue[i]; //take a semaphore from the queue
        semaphore_post(&mutex); // should not need this
        
        semaphore_post(&sem); //signal a voter
        
        semaphore_wait(&voter_done);//wait for a voter to get in
        semaphore_post(&poll_station_done);//signal so a voter can increment num_voted
    }
    
    while(num_voted != num_total_voters) {
        //let threads finish voting
        usleep(VOTE_TIME);
    }
    booths_open = 1;
    /*join threads*/
    for(i = 0; i < num_total_voters; i ++){
        //semaphore_destroy(queue[i]);
        pthread_join(threads[i], NULL);
    }
    for(i=0; i<num_booths; i++) {
        semaphore_destroy(&booths_mutex[i]);
    }
    /*destroy all other semaphores*/
    semaphore_destroy(&barrier);
    semaphore_destroy(&mutex);
    semaphore_destroy(&party_mutex); //may not need!
    semaphore_destroy(&rep_mutex);
    semaphore_destroy(&dem_mutex);
    semaphore_destroy(&ind_mutex);
    semaphore_destroy(&self_sem);
    semaphore_destroy(&poll_station_done);
    semaphore_destroy(&voter_done);
    semaphore_destroy(&voter_sem);
    semaphore_destroy(*queue);
    
    printf("---------------------+----------------------+--------------------------\n");

    /*free pointer variables*/
    if(rep_voters != NULL) {
        free(rep_voters);
        rep_voters = NULL;
    }
    if(dem_voters != NULL){
        free(dem_voters);
        dem_voters = NULL;
    }
    if(ind_voters != NULL){
        free(ind_voters);
        ind_voters = NULL;
    }
    if(queue != NULL) {
        free(queue);
        queue=NULL;
    }
    if(booths_mutex != NULL) {
        free(booths_mutex);
        booths_mutex = NULL;
    }
    if (string_booth_status != NULL) {
        free(string_booth_status);
        string_booth_status = NULL;
    }
    if (booth_status != NULL) {
        free(booth_status);
        booth_status = NULL;
    }
    pthread_exit(NULL);
    return 0;
}

void vote_sim(voter* cur_voter, int voter_pos){ /*passing in voter thread*/
    int i = 0;
    int rtnid = 0;
    usleep(random()%SIGN_IN_TIME);//simulate voting time randomly
    cur_voter->did_vote = 0; //set did_vote to true so it does not try to keep voting
    
}

void Sign_In_Desk(int voter_pos){
    usleep(random()%VOTE_TIME);//simulate sign in time randomly
}

void* Republican(void* thread_id) { //currently passing in threadID
    int tid = (intptr_t)thread_id;//pass in the thread id so can access the queue
    
    semaphore_wait(&mutex);//block other threads from accesing waiting vars
    int queue_pos = reps_waiting + dems_waiting + inds_waiting;
    semaphore_create(&self_sem, 0); //initialize self semaphore for this thread to 0
    queue[queue_pos] = &self_sem; //append to queue of semaphores
    reps_waiting++;
    //should not need to protect this print statement w/ mutex
    printf("Republican %4d      |-> %s <-| Waiting for polling station to open...\n", rep_voters[tid].voter_id, string_booth_status);
    semaphore_post(&mutex); //signal other threads that they can access
    semaphore_wait(&barrier); //blocks from here on out until last thread has arrived
    
    semaphore_post(&barrier); //unlocks critical section for rest of threads -- first thread is unlocked
    //once polling station is opened after main thread sleeps
    
    Sign_In_Desk(tid); //sign in once polling station is opened
    semaphore_wait(&mutex);
    printf("Republican %4d      |-> %s <-| Entering the polling station\n", rep_voters[tid].voter_id, string_booth_status);
    semaphore_post(&mutex);
    semaphore_post(&voter_sem); //signal a voter has arrived
    semaphore_wait(&self_sem); //wait until signal'd by the main thread

    semaphore_wait(&mutex);//wait until granted access to mutex
    if(cur_voting_party == -1) { //first one in -- need to set cur_voting_party
        cur_voting_party = 1;
    }
    if (cur_voting_party != 1) { //if the republicans are not voting make them wait
        semaphore_post(&mutex); //release mutex
        
        semaphore_wait(&rep_mutex); //wait until signaled by party_mutex
        semaphore_wait(&mutex);

    } else { //cur_voting party  == 1 (reps)
        //conditional to check to change voting party from reps to dems
        if( dems_waiting != 0 && (((dems_waiting/num_democrats) > (reps_waiting/num_republicans)) || dems_waiting > num_democrats - reps_waiting)) {//let some dems vote
            //take into account the party size too if can!
            cur_voting_party = 2;
            semaphore_post(&mutex);
            semaphore_post(&dem_mutex);//signal dem's to go
            semaphore_wait(&rep_mutex); //wait until signalled
        } else { //signal a waiting rep to vote
            semaphore_post(&mutex);
            semaphore_post(&rep_mutex);
        }
        semaphore_wait(&mutex); //when thread comes back, take mutex
        
    }

    while(dems_cur_voting != 0){ //wait until can vote
        semaphore_post(&mutex);
        //wait
        semaphore_wait(&mutex);
    }

    reps_cur_voting++;
    semaphore_post(&mutex); //unblock - signal
    int num_loops =0; //keep track of number of loops this thread is waiting to get in booth
    int i;
    /*wait in line for a booth*/
    while(rep_voters[tid].did_vote == 1) { //busy wait(wait in line)
        //change the above while to an if...? stops busy waiting?
        if(num_loops == 0) {
            semaphore_wait(&mutex);
            printf("Republican %4d      |-> %s <-| Waiting on a booth\n", rep_voters[tid].voter_id,
                   string_booth_status);
            semaphore_post(&mutex);
        }
        num_loops++;
        if(booths_open == TRUE && rep_voters[tid].did_vote == 1) { //this voter has yet to vote
            /* waiting for booth*/
            int booth_status_pos = 0;/*used to track where to put R's or I's or .'s*/
            for(i = 0; i < num_booths; i++) {//loop thru booths and check
                booth_status_pos = 1;//position in string_booth_status to place the char
                
                if(booth_status[i] == 1 && rep_voters[tid].did_vote == 1) { //open booth
                    semaphore_wait(&booths_mutex[i]); //control access to booth[i] vars - outside if?
                    semaphore_wait(&mutex);
                    
                    //semaphore_post(&mutex);
                    /*use semaphore_wait right away and change the booth status to decrease chance of
                     a voter checking the status as it is being changed */
                    booth_status_pos = booth_status_pos + (i*3);
                    booth_status[i] = 0;
                    string_booth_status[booth_status_pos] = 'R';
                    //need to block print statements with mutex
                    //3
                    printf("Republican  %3d in%3d|-> %s <-| Voting!\n", rep_voters[tid].voter_id,i, string_booth_status); //move to vote_sim?
                    semaphore_post(&mutex);
                    vote_sim(&rep_voters[tid], tid); //vote
                    
                    semaphore_wait(&mutex);//block access to booth_status[i] & num_voted
                    reps_waiting--;
                    string_booth_status[booth_status_pos] = '.';
                    printf("Republican  %3d      |-> %s <-| Leaving the polling station!\n",
                           rep_voters[tid].voter_id, string_booth_status); //move to end of vote_sim?
                    reps_cur_voting--;
                    semaphore_post(&mutex);
                    booth_status[i] = 1; //open the booth
                    semaphore_post(&booths_mutex[i]); //signal booth mutex[i] if there is one waiting - outside if?
                    semaphore_post(&voter_done);//signal at end?
                    semaphore_wait(&poll_station_done); //try w/out using this semaphore?
                    //signal outside of if?
                }
            }

            semaphore_wait(&mutex);

            if(reps_waiting > 0 && dems_waiting <= reps_waiting && (reps_cur_voting != 0 || cur_voting_party == 1)) { //see if there's more to vote or check other parties and if need to switch
                //to dems
                semaphore_post(&mutex);
                semaphore_post(&rep_mutex);
            } else if(dems_waiting != 0 && dems_waiting > reps_waiting){ //pre-emtpion clause -
                //signal dems to start voting
                cur_voting_party = 2;
                semaphore_post(&mutex);
                semaphore_post(&dem_mutex); //signal dem mutex
            }
        }
    }
    semaphore_wait(&mutex);
    reps_voted++;
    num_voted++;
    semaphore_post(&mutex);
    pthread_exit(NULL);
}

void* Democrat(void* thread_id) {
    int tid = (intptr_t)thread_id;
    
    printf("Democrat   %4d      |-> %s <-| Waiting for polling station to open...\n", dem_voters[tid-num_republicans].voter_id, string_booth_status);
    
    semaphore_wait(&mutex);//block other threads from accesing dems_waiting var
    semaphore_create(&self_sem, 0); //initialize self semaphore for this thread to 0
    int queue_pos = reps_waiting + dems_waiting + inds_waiting;
    queue[queue_pos] = &self_sem; //append to queue of semaphores
    dems_waiting++;
    semaphore_post(&mutex); //signal other threads that they can access?
    semaphore_wait(&barrier); //locks here until last thread has joined
    semaphore_post(&barrier);//unlocks critical section once last thread has joined
    
    Sign_In_Desk(tid); //simulate sign in time
    semaphore_wait(&mutex);
    printf("Democrat   %4d      |-> %s <-| Entering the polling station\n", dem_voters[tid-num_republicans].voter_id, string_booth_status);
    semaphore_post(&mutex);
    semaphore_post(&voter_sem); //signal a voter has arrived
    semaphore_wait(&self_sem); //wait until signal'd -- signal after checking voting party?
    
    semaphore_wait(&mutex);
    if(cur_voting_party == -1) { //first one in -- need to set cur_voting_party
        cur_voting_party = 2;
    }
    if (cur_voting_party != 2) { //dems are not voting
        semaphore_post(&mutex); //release mutex
        
        semaphore_wait(&dem_mutex); //wait until signaled by party_mutex
        semaphore_wait(&mutex);

    } else { //cur_voting_party == 2 (dems)
        if(reps_waiting != 0 && (((reps_waiting*num_republicans) > (dems_waiting*num_democrats)) ||
           reps_waiting > num_republicans - dems_waiting)) {//let some reps vote
            cur_voting_party = 1;
            semaphore_post(&mutex);
            semaphore_post(&rep_mutex);//signal reps's to go
            semaphore_wait(&dem_mutex); //wait until signalled
        } else { //signal a waiting dem to vote
            semaphore_post(&mutex);
            semaphore_post(&dem_mutex);
        }
        semaphore_wait(&mutex); //when thread comes back, take mutex
    }

    while(reps_cur_voting != 0) { //wait until can start to vote
        semaphore_post(&mutex);//give up mutex
        //wait...
        semaphore_wait(&mutex); //wait for it and check again
    }
    dems_cur_voting++;
    semaphore_post(&mutex);
    int num_loops = 0;
    while(dem_voters[tid-num_republicans].did_vote == 1) {//potential for busy waiting if while loop
        if(num_loops == 0) {
            semaphore_wait(&mutex);
            printf("Democrat    %3d      |-> %s <-| Waiting on voting booth\n", dem_voters[tid-num_republicans].voter_id, string_booth_status);
            semaphore_post(&mutex);
        }
        num_loops++;
        if(booths_open == TRUE && dem_voters[tid-num_republicans].did_vote == 1) { //check if need to vote
            //do not need booths_open var
            int i;
            int booth_status_pos = 0;/*used to track where to put R's or I's or .'s*/
            for(i = 0; i < num_booths; i++) {
                booth_status_pos = 1;
                if(booth_status[i] == 1 && dem_voters[tid-num_republicans].did_vote == 1) { //open booth
                    semaphore_wait(&booths_mutex[i]); //control access to booth[i] vars
                    semaphore_wait(&mutex);
                    
                    booth_status_pos = booth_status_pos + (i*3);
                    booth_status[i] = 0;
                    string_booth_status[booth_status_pos] = 'D';
                    printf("Democrat    %3d in%3d|-> %s <-| Voting!\n", dem_voters[tid-num_republicans].voter_id,i, string_booth_status); //move to vote_sim?
                    semaphore_post(&mutex);
                    
                    vote_sim(&dem_voters[tid-num_republicans], tid); //vote
                    semaphore_wait(&mutex);//block access to booth_status[i] & num_voted
                    
                    dems_waiting--;
                    string_booth_status[booth_status_pos] = '.';
                    printf("Democrat    %3d      |-> %s <-| Leaving the polling station!\n", dem_voters[tid-num_republicans].voter_id, string_booth_status); //move to end of vote_sim?
                    dems_cur_voting--;
                    semaphore_post(&mutex);
                    
                    booth_status[i] = 1;
                    semaphore_post(&booths_mutex[i]);
                    semaphore_post(&voter_done);
                    semaphore_wait(&poll_station_done);
                    //signal outside of if?
                }
            } //end for loop
            semaphore_wait(&mutex);
            if(dems_waiting > 0 && reps_waiting <= dems_waiting && (dems_cur_voting != 0||cur_voting_party==2)) { //see
                //if there's more to vote or check other parties and if need to switch
                semaphore_post(&mutex);
                semaphore_post(&dem_mutex);
            } else if (reps_waiting!=0 && reps_waiting > dems_waiting){ //pre-emtpion clause --
                //make it an if once working
                cur_voting_party = 1;
                semaphore_post(&mutex);
                semaphore_post(&rep_mutex); //signal rep party_mutex
            }
        } //end inner if
    }//end of outside while
    semaphore_wait(&mutex);
    dems_voted++;
    num_voted++;
    semaphore_post(&mutex);
    pthread_exit(NULL);
}

void* Independent(void* thread_id) {/*votes whenever an independent is signalled*/
    int tid = (intptr_t)thread_id;
    
    printf("Independent%4d      |-> %s <-| Waiting for polling station to open...\n", ind_voters[tid-(num_republicans + num_democrats)].voter_id, string_booth_status);
    
    semaphore_wait(&mutex);//block other threads from accesing waiting variable
    semaphore_create(&self_sem, 0); //initialize self semaphore for this thread to 0
    int queue_pos = reps_waiting + dems_waiting + inds_waiting;
    queue[queue_pos] = &self_sem; //append to queue of semaphores
    inds_waiting++;

    semaphore_post(&mutex); //signal other threads that they can access?
    semaphore_wait(&barrier); //locks anything past this after 1 thread
    semaphore_post(&barrier);//unlock critical section for next threads -- first unlock happens after
    //polling station is opened

    Sign_In_Desk(tid); //sign in after enterring poll station? should not matter
    semaphore_wait(&mutex);//control access to polling station/printing string_booth_status
    printf("Independent%4d      |-> %s <-| Entering the polling station\n", ind_voters[tid-(num_republicans +
                                                    num_democrats)].voter_id, string_booth_status);
    
    semaphore_post(&mutex);
    semaphore_post(&voter_sem); //signal a voter has arrived
    semaphore_wait(&self_sem); //wait until signaled after entering the polling station

    //print statements used to be here
    int num_loops =0;
    while(ind_voters[tid-(num_republicans+num_democrats)].did_vote == 1) {//busy wait (wait in line)
        if (num_loops == 0) {
            semaphore_wait(&mutex);
            printf("Independent%4d      |-> %s <-| Waiting on a voting booth\n", ind_voters[tid-(num_republicans + num_democrats)].voter_id, string_booth_status);
            semaphore_post(&mutex);
        }
        num_loops++;
        if(booths_open == TRUE && ind_voters[tid-(num_republicans+num_democrats)].did_vote == 1) {
            
            int i;
            int booth_status_pos = 0;/*used to track where to put R's or I's or .'s*/

            for(i = 0; i < num_booths; i++) {//check booths and maybe use a booth semaphore..?
                booth_status_pos = 1;
                if(booth_status[i] == 1 && ind_voters[tid-(num_republicans+num_democrats)].did_vote == 1) {
                    //open booth

                    semaphore_wait(&booths_mutex[i]); //control access to booth[i] vars
                    semaphore_wait(&mutex);
                    
                    booth_status_pos = booth_status_pos + (i*3);
                    booth_status[i] = 0;
                    string_booth_status[booth_status_pos] = 'I';
                    //block with mutex
                    printf("Independent %3d in%3d|-> %s <-| Voting!\n", ind_voters[tid-(num_republicans + num_democrats)].voter_id,i, string_booth_status); //move to vote_sim?
                    semaphore_post(&mutex);
                    
                    vote_sim(&ind_voters[tid-(num_republicans+num_democrats)], tid); //vote
                    semaphore_wait(&mutex);//block access to booth_status[i] & num_voted
                    
                    inds_waiting--;
                    string_booth_status[booth_status_pos] = '.';
                    printf("Independent %3d      |-> %s <-| Leaving the polling station!\n", ind_voters[tid-(num_republicans+num_democrats)].voter_id, string_booth_status); //move to end of vote_sim?
                    
                    semaphore_post(&mutex);
                    booth_status[i] = 1;
                    semaphore_post(&booths_mutex[i]);
                    semaphore_post(&voter_done);
                    semaphore_wait(&poll_station_done);
                    //signal outside of if?
                }
            }//end of for
            semaphore_wait(&mutex);
            //printf("cur_voting_party is %d\n", cur_voting_party);
            semaphore_post(&mutex);
            if (cur_voting_party == 1) { //pre-emtpion clause(s) to get parties to vote
                if (reps_waiting == 0 && dems_waiting > 0) {
                    cur_voting_party = 2;
                   
                    semaphore_post(&dem_mutex);
                    semaphore_post(&mutex);
                } else if(dems_waiting > reps_waiting){
                    cur_voting_party = 2;
                    semaphore_post(&dem_mutex);
                    semaphore_post(&mutex);
                } else {
                    semaphore_post(&mutex);
                }
            } else if (cur_voting_party == 2) { //pre-emption clause(s) to get reps to vote
                if (dems_waiting == 0 && reps_waiting > 0) {
                    cur_voting_party = 1;
                   
                    semaphore_post(&rep_mutex);
                    semaphore_post(&mutex);
                } else if (reps_waiting>dems_waiting){
                    cur_voting_party = 1;
                    
                    semaphore_post(&rep_mutex);
                    semaphore_post(&mutex);
                } else {
                    semaphore_post(&mutex);
                }
            } else { //will only hit this when first party in is an independent voting first
                semaphore_post(&mutex);
            }
            
        }//end of inner if
    } //end of outside while
    semaphore_wait(&mutex);
    inds_voted++;
    num_voted++;
    semaphore_post(&mutex);
    pthread_exit(NULL);
}

