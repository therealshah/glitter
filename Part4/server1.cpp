//
//  main.cpp
//  test
//
//  Created by Saqib Banna and Shahzaib on 3/16/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//
#include "Create.h"
#include "Follow.h"
#include "Delete.h"
#include "Tweet.h"
#include "Signin.h"
#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
//#include <stdio.h>       // perror, snprintf
//#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
//#include <string.h>      // strlen
//#include <strings.h>     // bzero
#include <time.h>        // time, ctime
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM,
// bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <utility>
// #include <pair>

using namespace std;



#define	MAXLINE		4096	// max text line length
#define	BUFFSIZE	8192    // buffer size for reads and writes
#define  SA struct sockaddr
#define	LISTENQ		1024	// 2nd argument to listen()
#define PORT_NUM        13002
#define FILE_NUM 1

void thefunction(istringstream& iss,int connfd);
void manageQueue(); // thread function
void finishedTask(const string& theoutput, char type, int connfd);

// THIS IS FOR CONCURRENCY
// queue<pair<istringstream,int>> line; // used to manage read/writes
queue<pair<string,int>> line; // used to manage read/writes
int readCounter = 0; // keep track iof how many reads are happening
bool isWriteHap = false;
mutex readLock; // used for the reads to increment the counter
mutex writeLock; // used to ck if writes are happening
mutex queueLock; // used to lock the queue
condition_variable isEmpty; // used for the bouncer thread
condition_variable doneWriting; // used to signal bouncer that read/write that a write has ended
condition_variable doneReading; // used to signal only the write that all reads are DONE

vector<int> serverList = {13002,13003,13004}; // all the servers
int mySeq;
bool isReplcating = false; // used to ck if we are multicast






int main(int argc, char **argv) {

    int			listenfd, connfd;  // Unix file descriptors
    struct sockaddr_in	servaddr;          // Note C use of struct
   // char      buff[MAXLINE];
    time_t      ticks;
  
    
    // 1. Create the socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Unable to create a socket");
        exit(1);
    }
    
    // 2. Set up the sockaddr_in
    
    // zero it.
    // bzero(&servaddr, sizeof(servaddr)); // Note bzero is "deprecated".  Sigh.
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET; // Specify the family
    // use any network card present
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PORT_NUM);	// daytime server
    
    // 3. "Bind" that address object to our listening file descriptor
    if (::bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Unable to bind port");
        exit(2);
    }
    
    // 4. Tell the system that we are going to use this sockect for
    //    listening and request a queue length
    if (listen(listenfd, LISTENQ) == -1) {
        perror("Unable to listen");
        exit(3);
    }

    thread bouncer(manageQueue); // bounder for the read/write
    unique_lock<mutex> ql(queueLock,defer_lock); // set up unique lock for queue lock, but dont lock now
    
    for ( ; ; ) {
        // 5. Block until someone connects.
        //    We could provide a sockaddr if we wanted to know details of whom
        //    we are talking to.
        //    Last arg is where to put the size of the sockaddr if
        //    we asked for one
        fprintf(stderr, "Ready to connect. Thread2\n");
        if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
            perror("accept failed");
            exit(4);
        }
        // if we made it here, we make a new thread to handle this user's request
      
       
        fprintf(stderr, "Connected\n");
        
        // We had a connection.  Do whatever our task is.
        //cout <<" after ticks";
        ticks = time(NULL);
        //cout <<" after ticks";
        // snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        char thingy[MAXLINE];
        read(connfd,thingy,MAXLINE);
        cout <<"Main:Pushing into queue\n";
        // we basically put this request\n into the queue
        // Then we signal bouncer to take care of it
        cout << "Main:Getting queuelock\n";
        ql.lock();
        line.push(make_pair(thingy,connfd));
        ql.unlock(); // unlock the queue lock
        cout << "Main:Unlocking and signaling\n";
        isEmpty.notify_all(); // signal the waiting bouncer thread

        // string theoutput = thefunction(thingy);
        memset(thingy, 0, sizeof thingy); // Need to clear the buffer after use
 
    }
}


/*----------------------------------------------------------------------------
    -- This function is is basically used by he bouncer thread
    -- Manages reeads/writrs
    // If read is happening, allows all next reads to go
    -- If write is happening, blocks everything and allows the write to go in
    -- after all reads are gone
------------------------------------------------------------------------------*/
void manageQueue(){
    //vector<thread> vect;
    unique_lock<mutex> ul(queueLock,defer_lock); // set up the unique lock, but don't lock right now
    unique_lock<mutex> wl(writeLock,defer_lock); // for write lock
    unique_lock<mutex> rl(readLock, defer_lock); // for read lock
    while (true){
        //     // first ck if queue is empty or not
        cout << "ManageQueue:Locking queue"<<endl;
        ul.lock(); // lock the queue
        while (line.empty()){
            cout << "ManageQueue:Sleeping on queue signal\n";
            isEmpty.wait(ul); // wait until there is an element
            cout << "ManageQueue:Waking up on queue signal\n";
        } 
        // now ck the front of the queue
        cout << "ManageQueue:We woke up. Popping from qeueu\n";
        auto req = line.front(); // get the top element
        line.pop(); // pops the element
        ul.unlock(); // unlock to all the main to push more requests
        cout << "ManageQueue:Unlocked the queue\n";
        // now ck if the request can go
        istringstream iss(req.first); // get the string and make a stringstream
        string requestType; // ck if read or write request
        getline(iss,requestType,':'); 
        /*---------------------------------
             -- if it's a read type:
                1. Ck if there's a write
                2. If yes, sleep until no write
                3. otherwise, incremenet count and go
        ------------------------------------*/ 
        if (requestType == "r"){
            cout << "ManageQueue:Read request\n";
            cout << "Getting write lock\n";
            wl.lock(); // get write lock and increment ur count
            // while a write is happening, sleep until u are notified
            while (isWriteHap){
                cout << "ManageQueue:Sleeping on write signal\n";
                doneWriting.wait(wl); 
                cout << "ManageQueue:Waking up on write signal\n";
            }
            cout << "ManageQueue:Getting read lock\n";
            rl.lock(); // lock the read so we can increment
            readCounter++; // increment the read counter
            rl.unlock(); // unlock the read lock
            wl.unlock(); // unlock the write lock
            cout << "ManageQueue:Unlocking both read lock and write lock\n";

            // now we are good to handle the request
            thread t = thread(thefunction,ref(iss),req.second); // pass stringstream and confd

        }
        /*---------------------------------
          else it's a write  type:
            1. Ck the counter, and see if there are any reads
            2. sleep until no reads
        ------------------------------------*/ 
        else{
            cout << "ManageQueue:Write request\n";
            cout << "ManageQueue:Getting write lock\n";
            wl.lock(); // make sure no other write AND READ can access the files
            // while a write is happening, sleep until u are notified
            while (isWriteHap) doneWriting.wait(wl); 
            cout << "ManageQueue:Getting read lock\n";
            rl.lock(); // acquire the increment lock
            // if there are reads happening, wait until the reads are done
            // dont let go of ur write lock bc this is the first lock every
            // thread needs to get
            // SO its fair
            while (readCounter != 0){
                cout << "ManageQueue:Sleeping on read signal\n";
                doneReading.wait(rl);
                cout << "ManageQueue:Waking up on read signal\n";
 
            } 
            // while (readCounter != 0){
            //     // unlock, sleep fir a bit and try again
            //     readLock.unlock();
            //     this_thread::sleep_for(chrono::milliseconds(100)); // sleep
            //     readLock.lock(); // acquire the lock before we access SHARED RESCOURCE
            // }
            isWriteHap = true; // we are writing now
            // unlock the read and write and go
            rl.unlock();
            wl.unlock();
            cout << "ManageQueue:Unlocking both read lock and write lock\n";
            // if it is equal to w (meaning its from FE, we will multicast AKA we are not replicating)
            // else the primary multicasted the update to me and i will not multicast
            bool replicate = (requestType=="w")? false:true;
            thread t = thread(thefunction,ref(iss),req.second,replicate); // pass stringstream and confd
        }
        // else{
        //     cerr << "WHAT?";
        //     exit(5);
        // }

        t.detach(); // detach itself from main thread/ plus it won't get cleaned up after somehting happens


     } // end of while loop
}


/*----------------------------------------------------------------------------
    -- This function is is basically replicates the update to every server except me
------------------------------------------------------------------------------*/
void replicate(const string& command){

}

/*----------------------------------------------------------------------------
    -- This function is is basically ends the request. It sends back mssg to the
    -- client and it closes the connection
    -- It also:
        -- decrements the readCounter if the type is r
        -- sets isWriteHap to false if type w
    -- @param:
        - output - return mssg
        - type
------------------------------------------------------------------------------*/
void finishedTask(const string& theoutput, char type, int connfd,const string& command = "empty"){
    // if read, decrement counter
    // if read counter is 0, signal doneReading


    if (type == 'r'){
        cout << "FinishedTask:Getting read lock\n";
        unique_lock<mutex> rl(readLock);
        // if we are the last read, signal and unlock
        if (--readCounter == 0){
            cout << "Signaling\n";
            doneReading.notify_all();
        }
        rl.unlock(); // unlock bc we don't need it
        cout << "FinishedTask:unlocking read lock\n";
        char      buff[MAXLINE]; // used to return the output
        snprintf(buff, sizeof(buff), "%s", theoutput.c_str()); // store the output into the buffer
       // printf(buff);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }
        
        // 6. Close the connection with the current client and go back
        //    for another.
        close(connfd);
    }
    // if write, change iswriteHap to false and notify all
    else if (type == 'w'){
        cout << "FinishedTask:Getting write lock\n";

        // only multicast if its not a direct command from FE
        // if command == empty => implies primary
        mySeq++;
        command = "replicate:" + mySeq + ":" + command; // type is request and append seq #
        if (command != "empty")
            replicate(command);
        unique_lock<mutex> wl(writeLock);
        // if we are the last read, signal and unlock
        isWriteHap = false; // no more writes
        cout << "FinishedTask:Signaling done writning\n";
        doneWriting.notify_all(); // we are done writing
        wl.unlock(); // unlock bc we don't need it
        cout << "FinishedTask:unlocking write lock\n";
        // everything else is just return the output the user and close connection
        char      buff[MAXLINE]; // used to return the output
        snprintf(buff, sizeof(buff), "%s", theoutput.c_str()); // store the output into the buffer
        //printf(buff);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }

        // before closing, we will multicast 
        
        // 6. Close the connection with the current client and go back
        //    for another.
        close(connfd);
    }
    else{
        cerr << "YOU MESSEDUP ";
        exit(6);
    }
}


/*----------------------------------------------------------------------------
    -- This function is passed in the request that was receieved from the client
    -- Everything is split by a ':' (Colon)
    -- The first field is the action the server wants and afyer that the data (most likely the userID or search fields)
------------------------------------------------------------------------------*/
void thefunction(istringstream& orgIss,int connfd,bool replicate = false){

    // get whole string which will be used to multicast
    string command;
    getline(orgIss,command); // this basically reads the whole string
    istringstream iss(command); // make a new stringsteam with the command., this way we have the orignal command

    string thefunc;
    getline(iss,thefunc,':');
    //cout << "From client: " << theinput << endl;
   //cout << thefunc<<endl;
    // Now we will parse for functionality and call the right function to handle the request
    char TWEET_FILE [] = "tweets_server1.txt";
    char  USERS_FILE [] =  "users_server1.txt";
    char  FRIENDS_FILE [] =  "friends_server1.txt";

    

    if(thefunc == "create"){
        string id, name, password;
        getline(iss,id,':');
        getline(iss,name,':');
        getline(iss,password,':');
        string output =  create(id,name,password,TWEET_FILE,USERS_FILE,FRIENDS_FILE); // get output
        command = thefunc + 
        finishedTask(output,'w',connfd,command ); // weite
    } 
    else if(thefunc == "signin"){ // read
        string id, password;
        getline(iss,id,':');
        getline(iss,password,':');
        string output =  signin(id,password,USERS_FILE);
        finishedTask(output,'r',connfd);
        
    } 
    else if(thefunc == "gettweet"){ // read
        string id;
        getline(iss,id,':');
        string output =  getTweets(id,TWEET_FILE);
        finishedTask(output,'r',connfd);
    } 
    else if(thefunc == "tweet"){ // read
        string id, tweet,timestamp;
        getline(iss,id,':');
        getline(iss,tweet,':');
        getline(iss,timestamp,':'); // this is the timestamp
        writeTweet(id, tweet,timestamp,TWEET_FILE);
        string output =  getTweets(id,TWEET_FILE);
        finishedTask(output,'w',connfd,command);

    } 
    else if(thefunc == "searchPeople"){ // read
        // finding all of the people
        string username,personName;
        // get the username and person to look up ( note we will look up using both name and userid)
        getline(iss,username,':');
        getline(iss,personName,':');
        string output =  findPeople(username,personName,USERS_FILE);
        finishedTask(output,'r',connfd);
    } 
    else if (thefunc == "searchFollowList"){ // read
        string username,personName;
        getline(iss,username,':');
        getline(iss,personName,':');
        string output =  getFollowing(username,personName,FRIENDS_FILE); // overloaded function to get the people that are following based on the personName
        finishedTask(output,'r',connfd);
    }
    else if(thefunc == "delete"){ //read
        string id;
        getline(iss,id);
        deleteAccount(id,TWEET_FILE,USERS_FILE,FRIENDS_FILE);
        finishedTask("delete happening",'w',connfd,command); // we puut dummy data in for output bc delete doesnt do anything
    } 
    else if (thefunc == "getFollowing"){ // read
        // get all of my friends
        string username; // used to hold the username
        getline(iss,username,':'); // get the username
        string output =  getFollowing(username,FRIENDS_FILE); // gets all the people i am following
        finishedTask(output,'r',connfd);
    }
    else if (thefunc == "unfollow"){ //write
        // remove this from my follow list
        string username,friendname; // remove the friend from my friends list
        getline(iss,username,':'); // my name
        getline(iss,friendname,':'); // friendname
        string output = unfollow(username,friendname,FRIENDS_FILE);
        finishedTask(output,'w',connfd,command);
    } 
    else if (thefunc == "follow"){ //write
        string username, personName;
        // get my id and the person i wanna unfollow's id
        getline(iss,username,':');
        getline(iss,personName,':');
        string output =  follow(username,personName,FRIENDS_FILE);
        finishedTask(output,'w',connfd,command);
    }
    else if (thefunc == "searchPersonTweet"){ //read
        // we are getting a particular person's tweets
        string personName;
        getline(iss,personName,':'); // get the name of person we are searching for
        string output=  searchPersonTweet(personName,TWEET_FILE);
        finishedTask(output,'r',connfd);
    }
    else {
        cerr << " ERROR IN THE FUNC";
    }
};






