//
//  main.cpp
//  test
//
//  Created by Saqib Banna and Shahzaib on 3/16/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//

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

void thefunction(istringstream& iss,int connfd);
string create(const string& id, const string& username, const string& password);
string signin(const string& id, const string& password);
string getTweets(const string& id);
void writeTweet(const string& id, const string& tweets,const string& timestamp);
string deleteAccount(const string& id);
bool removeIDfromfile(const string& clientID, const char theFile[]);
bool removeIdFromYourFollowers(const string& clientID);
string getFollowing(const string& username);
string getFollowing(const string& username,const string& personName);
string unfollow(const string& username,const string& friendname);
string findPeople(const string& username, const string& personName);
void readUsers(vector<pair<string,string>>& users);
string follow(const string& myUserName, const string& personName);
string searchPersonTweet(const string& personName);
void manageQueue(); // thread function
void finishedTask(const string& theoutput, char type, int connfd);

// THIS IS FOR CONCURRENCY
queue<pair<istringstream,int>> line; // used to manage read/writes
int readCounter = 0; // keep track iof how many reads are happening
bool isWriteHap = false;
mutex readLock; // used for the reads to increment the counter
mutex writeLock; // used to ck if writes are happening
mutex queueLock; // used to lock the queue
condition_variable isEmpty; // used for the bouncer thread
condition_variable doneWriting; // used to signal bouncer that read/write that a write has ended
condition_variable doneReading; // used to signal only the write that all reads are DONE





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
        cout <<" after ticks";
        ticks = time(NULL);
        cout <<" after ticks";
        // snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        char thingy[MAXLINE];
        read(connfd,thingy,MAXLINE);
        cout <<" Pushing into queue";
        // we basically put this request into the queue
        // Then we signal bouncer to take care of it
        queueLock.lock();
       // line.push(make_pair(istringstream(thingy),connfd));
        queueLock.unlock(); // unlock the queue lock
        isEmpty.notify_all(); // signal the waiting bouncer thread

        // string theoutput = thefunction(thingy);
        memset(thingy, 0, sizeof thingy); // Need to clear the buffer after use

        // snprintf(buff, sizeof(buff), "%s", theoutput.c_str());
        // printf(buff);
        // int len = strlen(buff);
        // if (len != write(connfd, buff, strlen(buff))) {
        //     perror("write to connection failed");
        // }
        
        // // 6. Close the connection with the current client and go back
        // //    for another.
        // close(connfd);
        // memset(buff, 0, sizeof buff); // Need to clear the buffer after use

 
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
    vector<thread> vect;
    unique_lock<mutex> ul(queueLock,defer_lock); // set up the unique lock, but don't lock right now
    unique_lock<mutex> wl(writeLock,defer_lock); // for write lock
    unique_lock<mutex> rl(readLock, defer_lock); // for read lock
    while (true){
        // first ck if queue is empty or not
        ul.lock(); // lock the queue
        while (line.empty()) isEmpty.wait(ul); // wait until there is an element
        // now ck the front of the queue
        auto req = line.front(); // get the top element
        line.pop(); // pops the element
        ul.unlock(); // unlock to all the main to push more requests
        // now ck if the request can go
        istringstream iss = req.first; // get the string stream
        string requestType; // ck if read or write request
        getline(iss,requestType,':'); 
        /*---------------------------------
             -- if it's a read type:
                1. Ck if there's a write
                2. If yes, sleep until no write
                3. otherwise, incremenet count and go
        ------------------------------------*/ 
        if (requestType == "r"){
            wl.lock(); // get write lock and increment ur count
            // while a write is happening, sleep until u are notified
            while (isWriteHap) doneWriting.wait(wl); 
            rl.lock(); // lock the read so we can increment
            readCounter++; // increment the read counter
            rl.unlock(); // unlock the read lock
            wl.unlock(); // unlock the write lock

            // now we are good to handle the request
            vect.push_back(thread(thefunction,iss,req.second)); // pass stringstream and confd

        }
        /*---------------------------------
          if it's a write  type:
            1. Ck the counter, and see if there are any reads
            2. sleep until no reads
        ------------------------------------*/ 
        else if (requestType == "w"){
            wl.lock(); // make sure no other write AND READ can access the files
            // while a write is happening, sleep until u are notified
            while (isWriteHap) doneWriting.wait(wl); 
            rl.lock(); // acquire the increment lock
            // if there are reads happening, wait until the reads are done
            // dont let go of ur write lock bc this is the first lock every
            // thread needs to get
            // SO its fair
            while (readCounter != 0) doneReading.wait(rl);
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
            vect.push_back(thread(thefunction,iss,req.second)); // pass stringstream and confd
        }
        else{
            cerr << "WHAT?";
            exit(5);
        }


    }
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
void finishedTask(const string& theoutput, char type, int connfd){
    // if read, decrement counter
    // if read counter is 0, signal doneReading
    if (type == 'r'){
        unique_lock<mutex> rl(readLock);
        // if we are the last read, signal and unlock
        if (--readCounter == 0){
            doneReading.notify_all();
        }
        rl.unlock(); // unlock bc we don't need it
        char      buff[MAXLINE]; // used to return the output
        snprintf(buff, sizeof(buff), "%s", theoutput.c_str()); // store the output into the buffer
        printf(buff);
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
        unique_lock<mutex> wl(writeLock);
        // if we are the last read, signal and unlock
        isWriteHap = false; // no more writes
        doneWriting.notify_all(); // we are done writing
        wl.unlock(); // unlock bc we don't need it
        // everything else is just return the output the user and close connection
        char      buff[MAXLINE]; // used to return the output
        snprintf(buff, sizeof(buff), "%s", theoutput.c_str()); // store the output into the buffer
        printf(buff);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }
        
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
void thefunction(istringstream& iss,int connfd){
    string thefunc;
    // istringstream iss(theinput);
    getline(iss,thefunc,':');
    //cout << "From client: " << theinput << endl;
   //cout << thefunc<<endl;
    // Now we will parse for functionality and call the right function to handle the request
    if(thefunc == "create"){
        string id, name, password;
        getline(iss,id,':');
        getline(iss,name,':');
        getline(iss,password,':');
        string output =  create(id,name,password); // get output
        finishedTask(output,'w',connfd); // weite

    } 
    else if(thefunc == "signin"){ // read
        string id, password;
        getline(iss,id,':');
        getline(iss,password,':');
        string output =  signin(id,password);
        finishedTask(output,'r',connfd);
        
    } 
    else if(thefunc == "gettweet"){ // read
        string id;
        getline(iss,id,':');
        string output =  getTweets(id);
        finishedTask(output,'r',connfd);
    } 
    else if(thefunc == "tweet"){ // read
        string id, tweet,timestamp;
        getline(iss,id,':');
        getline(iss,tweet,':');
        getline(iss,timestamp,':'); // this is the timestamp
        writeTweet(id, tweet,timestamp);
        string output =  getTweets(id);
        finishedTask(output,'r',connfd);

    } 
    else if(thefunc == "searchPeople"){ // read
        // finding all of the people
        string username,personName;
        // get the username and person to look up ( note we will look up using both name and userid)
        getline(iss,username,':');
        getline(iss,personName,':');
        string output =  findPeople(username,personName);
        finishedTask(output,'r',connfd);
    } 
    else if (thefunc == "searchFollowList"){ // read
        string username,personName;
        getline(iss,username,':');
        getline(iss,personName,':');
        string output =  getFollowing(username,personName); // overloaded function to get the people that are following based on the personName
        finishedTask(output,'r',connfd);
    }
    else if(thefunc == "delete"){ //read
        string id;
        getline(iss,id);
        deleteAccount(id);
        finishedTask("delete happening",'w',connfd); // we puut dummy data in for output bc delete doesnt do anything
    } 
    else if (thefunc == "getFollowing"){ // read
        // get all of my friends
        string username; // used to hold the username
        getline(iss,username,':'); // get the username
        string output =  getFollowing(username); // gets all the people i am following
        finishedTask(output,'r',connfd);
    }
    else if (thefunc == "unfollow"){ //write
        // remove this from my follow list
        string username,friendname; // remove the friend from my friends list
        getline(iss,username,':'); // my name
        getline(iss,friendname,':'); // friendname
        string output = unfollow(username,friendname);
        finishedTask(output,'w',connfd);
    } 
    else if (thefunc == "follow"){ //write
        string username, personName;
        // get my id and the person i wanna unfollow's id
        getline(iss,username,':');
        getline(iss,personName,':');
        string output =  follow(username,personName);
        finishedTask(output,'w',connfd);
    }
    else if (thefunc == "searchPersonTweet"){ //read
        // we are getting a particular person's tweets
        string personName;
        getline(iss,personName,':'); // get the name of person we are searching for
        string output=  searchPersonTweet(personName);
        finishedTask(output,'r',connfd);
    }
    else {
        cerr << " ERROR IN THE FUNC";
    }

};


// Function for Creating new accounts
string create(const string& id, const string& username, const string& password){
    string line, token;
    fstream idfile("users.txt");
    if(!idfile.is_open()){cerr<< "Users.txt file did not open, create failed \n";}
    
    // Does the ID already exist? If so mark it and leave nested loop.
    bool idexists = 0;
    while(getline(idfile, line)){
        istringstream spilt(line);
        token = "";
        while(getline(spilt, token, ':')){
            if(id==token){
                idexists = 1;
                goto afterreading;
            }
            break;
        }
    }
    
    afterreading:
    idfile.close();
    if (idexists){ // ID already exists, send back exists
        return "exists";
    } 
    else {
        // ID doesn't exist, add it.
        ofstream userFile;
        userFile.open ("users.txt",ios::app);
        userFile << id << ":" << username << ":" << password << "\n";
        userFile.close();
        
        // Add them to tweet file
        ofstream tweetFile;
        tweetFile.open ("tweets.txt",ios::app);
        tweetFile << id << "\n";
        tweetFile.close();
        
        // Add them to friends file
        ofstream friendsFile;
        friendsFile.open ("friends.txt",ios::app);
        friendsFile << id << "\n";
        friendsFile.close();
        
        return "newaccount";
    }
};

// Function for Signing into Accounts
string signin(const string& id, const string& password){
    string line, idOnFile, nameOnFile, passOnFile;
    ifstream userfile("users.txt");
    if(!userfile.is_open()){cerr << "Users.txt did not open, Sign in failed \n";}
    
    // Does the ID already exist?
    bool idpassvalid = 0;
    while(getline(userfile, line)){
        istringstream spilt(line);
        idOnFile = "";
        while(getline(spilt, idOnFile, ':')){
            if(id==idOnFile){ //ID exists, does the password match?
                getline(spilt, nameOnFile, ':');
                getline(spilt, passOnFile, ':');
                if(passOnFile==password){
                    //Password match, success
                    idpassvalid = 1;
                }
                goto afterreading;
            }
            break;
        }
    }
    
    afterreading:
    userfile.close();
    if (idpassvalid == 0){
        return "invalid";
    } else {
        return "success";
    }
};

// Go through the tweet file, find the id, put the tweets
string getTweets(const string& id){
    string line, idOnFile, tweet;
    ifstream tweetfile("tweets.txt");
    if(!tweetfile.is_open())
        {
            cout<< "Tweet file did not open \n";
            return "error";
        }

    while(getline(tweetfile, line)){
        istringstream iss(line);
        // Put the id from the line into idOnFile
        getline(iss,idOnFile,':'); // get the id of this person
        if (id == idOnFile){
            tweetfile.close(); // close the file
            return "success:" +  line; // we found this person's tweets
        }
    }
    
    tweetfile.close();
    return "notfound"; // not found
};

void writeTweet(const string& id, const string& tweet, const string& timestamp){
    string str, username;
    char nameOfTweetFile[] = "tweets.txt";
    char tempTweetFile[] = "tweetstemp.txt"; // tempfile to write into

    ifstream in_file(nameOfTweetFile); // Reach from original tweet file
    if(!in_file){ cerr << "Could not open input file for tweets\n";}
    
    ofstream out_file(tempTweetFile); // Write to temp file
    if(!out_file){cerr << "Could not create output file for tweets\n";}
    
    while (getline(in_file, str)) {
        istringstream iss(str);
        getline(iss, username, ':');    // Put the id from the line into idOnFile
       // cout <<"DEBUGGING"<< username<<endl;
        if(id==username){ // If id matches add tweet to end
            out_file << str << ":"<<tweet << ":" << timestamp<<"\n";
        } else { // Otherwise just copy the string
            out_file << str << "\n";
        }
    }
    in_file.close();
    out_file.close();
    
    if( remove( nameOfTweetFile ) != 0 ) // Delete old file
        perror( "Error deleting file" );
    else{
        if ( rename( tempTweetFile , nameOfTweetFile ) != 0 ) // Replace old file
            perror( "Error renaming file" );
    }
    
}

// This method deletes the user from all files by calling other methods
string deleteAccount(const string& id){
    char nameOfTweetFile[] = "tweets.txt";
    char nameOfFriendFile[] = "users.txt";
    char nameOfUserFile[] = "friends.txt";


    // Clever use of one function to delete everything from a specific file
    if(!removeIDfromfile(id, nameOfTweetFile)){
        perror( "Error deleting user from tweets" );
    }
    if(!removeIDfromfile(id, nameOfFriendFile)){
        perror( "Error deleting user from Friend" );
    }
    if(!removeIDfromfile(id, nameOfUserFile)){
        perror( "Error deleting user from User" );
    }
    if(!removeIdFromYourFollowers(id)){
        perror( "Error deleting user from those following" );
    }
    return "successfulDelete";
}

// This method removes any line that start with the client ID in any file
bool removeIDfromfile(const string& clientID, const char theFile[]){
    char nameOfTempFile[] = "temp.txt";
    string str, idOnFile;

    ifstream in_file(theFile);  // Open the original file to read from
    if(!in_file){cerr << "Could not open input file for tweets\n";}
    
    ofstream out_file(nameOfTempFile);  // Create a temp file to write into
    if(!out_file){cerr << "Could not create output file for Temp\n";}
    
    while (getline(in_file, str)) { // Read original file line by line
        istringstream iss(str);
        getline(iss, idOnFile, ':'); // Get the id on each line
        if(clientID!=idOnFile){ // Copy all except line with matching id
            out_file << str << "\n";
        }
    }
    in_file.close();
    out_file.close();
    
    //Now basically remove the old file and rename the new file
    if( remove( theFile ) != 0 ) // Delete the old file
        cerr << "Error deleting file: " << theFile << endl;
    else{
        if ( rename( nameOfTempFile , theFile ) != 0 ) // Rename the new file
            cerr << "Error renaming file: " << nameOfTempFile << endl;
        return 1; // Everything worked!
    }
    return 0; // If reached theres a failure
}

// This method removes your ID from everyone following you
// The file is copied over without any instance of the clientID in parameter
// A 1 is returned if the process is successful
bool removeIdFromYourFollowers(const string& clientID){
    char nameOfTempFile[] = "temp.txt";
    char nameOfFriendFile[] = "friends.txt";
    string str, idsInFile;
    
    ifstream in_file(nameOfFriendFile);  // Open the original file to read from
    if(!in_file){
            cerr << "Could not open input file for tweets\n";
            return 0;
     }
    
    ofstream out_file(nameOfTempFile);  // Create a temp file to write into
    if(!out_file){
        cerr << "Could not create output file for Temp\n";
        return 0;
    }
    
    while (getline(in_file, str)) { // Read original file line by line
        istringstream iss(str);
        getline(iss,idsInFile,':'); // get my user id and write that to the file
        out_file<<idsInFile; // we wrote our id
        while(getline(iss, idsInFile, ':')){ // Get the id on each line
            if(clientID!=idsInFile) // Copy all except line with matching id
                out_file << ":"<< idsInFile; // write our id
        }
        out_file << "\n";
    }
    in_file.close();
    out_file.close();
    
    if( remove( nameOfFriendFile ) != 0 ) // Delete the old file
        cerr << "Error deleting file: " << nameOfFriendFile << endl;
    else {
        if ( rename(nameOfTempFile,nameOfFriendFile) ==0) // Rename the new file
            return 1; // Everything worked!
        else
            cerr << "Error renaming file: " << nameOfTempFile << endl;
    }
    return 0;
}


/*
 -- This method gets all the people i am following
 -- Will return an error code if an error occurs otherwise it will return success with the user friends
 */
string getFollowing(const string& username){
    
    // open the file for reading and find my username
    ifstream ifs("friends.txt"); // open the file for reading
    if (!ifs){
        cerr << "Couldn't open file\n";
        return "error";
        //exit;
    }
    // keep loopring through until we hit out username
    // the data is separated by :
    string line,user;
    while (getline(ifs,line)){
        // while we have a line to read
        // ck the user
        int pos = line.find_first_of(':'); // basically get the position of where the username ends
        user = line.substr(0,pos); // get the username
        if (user == username){
            //cout <<" line" << line<<endl;
            ifs.close();
            return "success:" + line; // return this guys following list
        }
    }
    ifs.close();
    return "error"; // otherwise we didn't find this guys friends list, so just return error
}

/*
    -- This has the same task as the getFollowing method but only returns
    -- the people that the same username
    --  makes use of the getFollowing method

*/
string getFollowing(const string& username, const string& personName){
    // call the other getFollowing method to get my followlist
    string followList = getFollowing(username);
    // ck if it was sucessfull
    if (followList == "error")
        return followList; // basically return if we encoutnered an error
    // otherwise parse the followList and get all the people that have either the username 
    string newFollowList = "success"; 
    istringstream iss(followList);
    string token;
    getline(iss,token,':'); // get the status code
    getline(iss,token,':'); // get my username
    while (getline(iss,token,':')){
        // parse each word and get the person that matches that userId
        if (token == personName ){
            newFollowList = newFollowList + ":"+ token; // add it to the followList
            return newFollowList; // return becase we are guareenteed to not find someone else with the same id
        }
    }

    return newFollowList; // we didn;t find anyone, so return empty list
}


/*
 -- This method accepts a username and a friendname
 -- basically removes the friend from the followlist of username
 
 */
string unfollow(const string& username,const string& personName){

    char nameOfTempFile[] = "temp.txt";
    char nameOfFriendFile[] = "friends.txt";
    string str, idsInFile;
    string output = "success"; // we will also keep track of our friends so we don't have to reopen the file to find our friends
    
    ifstream in_file(nameOfFriendFile);  // Open the original file to read from
    if(!in_file)
        {
            cerr << "Could not open input file for unfollowing a person\n";
            return "error";
        }
    
    ofstream out_file(nameOfTempFile);  // Create a temp file to write into
    if(!out_file)
        {
            cerr << "Could not create output file for Temp in unfollow method\n";
            return "error";
        }
    //cout << "Want to remove " << personName<<endl;
    while (getline(in_file, str)) { // Read original file line by line
        //cout << "Str being read is " << str << endl;
        istringstream iss(str); // parse the string that we read in
        getline(iss,idsInFile,':');// get the id of this person
        if(username==idsInFile){ // Copy all except line with matching id
            // copy all the people over except the one we want to unfollow
            string token;
            output = output +":"+ username; // add the username
           // cout << "Current output " <<output<<endl;
            out_file << username; // write the id first
            while (getline(iss,token,':')){
                 //cout << "Token being read in "<<token<<endl;
                if (token != personName){ // write the person only if it doesn't equal the person we want to unfollow
                    out_file << ":"<<token;
                    //cout << "Comparing " << token<< " and " << personName<< " " << (personName == token)<<endl;
                    //cout << "writing friend "<<token<<endl;
                    output = output + ":" + token; // add this to the output string as well
                }
            }
            out_file<<endl; // write a new line
        }
         
        
        else{
            //cout << "Writing full: "<<str<<endl;
            out_file<<str<<endl;
        }
    }
        
    in_file.close();
    out_file.close();
    
    if( remove( nameOfFriendFile ) != 0 ){ // Delete the old file
        cerr << "Error deleting file: " << nameOfFriendFile << endl;
        return "error";
    }
    else {
        if ( rename(nameOfTempFile,nameOfFriendFile) !=0) // Rename the new file
            return "error";
    }

   // cout << "returning " <<output<<endl;
   // cout << getFollowing(username)<<endl;
    return output;
}



// Finds all the people that are friends of the username
string findPeople(const string& myUserName, const string& personName){
    string returnMssg = "success"; // this will hold the return mssg  
    // first read in all of my friends
    unordered_map<string,vector<string>> map; // hold all the followers of a person
    string myFollowing = getFollowing(myUserName); // get all the people that i am following
    vector<string> followVect; // this will hold all the people i am following
    istringstream followList(myFollowing); // parse for all of my friends
    string token;
    getline(followList,token,':'); // read in the status code
    while (getline(followList,token,':'))
        followVect.push_back(token); // push back the name in the follow vector
    // now read in all users
    ifstream ifs("users.txt");
    if (!ifs){
        cout << "Can't open users.txt file\n";
        return "error";
    }

    // now read each line by line and only add those to the mssg that  match personName and are not me
    string line;
    while (getline(ifs,line)){
        // parse the line
        // the data is stored as = username:name:pass 
        // we only care about name and username
        // parse for the name and username
        istringstream iss(line); 
        string name,username;
        getline(iss,username,':'); // get name
        getline(iss,name,':');  // get username

        // only add this to the string only if the name or userid matches the string and the username is not mine
        // also if i am not already following this person
        if (username != myUserName && (username == personName || name == personName))
        {    // make sure we are not already following this person
            if (find(followVect.begin(),followVect.end(),username) == followVect.end())
                returnMssg = returnMssg + ":" + username; // only return the username
        }
    }

    ifs.close(); // close the file
    return returnMssg; // return the mssg
}


// This method will bascically add the person to the friend vector
string follow(const string& myUserName, const string& personName){

    // we will read from the input file and store in a temp output file. If we encounter the username, we will simply
    // append it's data to the end of the string
    char inputFile [] = "friends.txt";
    char outputFile [] = "temp.txt";
    ifstream inFile(inputFile);
    if (!inFile){
        cout << "Can't open friends file to follow someone";
        return "error";
    }
    // create a output file
    ofstream outFile(outputFile); // temp file
    if (!outFile){
        cout << "Can't create temp file in the function follow";
        return "error";
    }

    // now read the data from infile and put it in the outFile
    // If the id matches username, simply append to the end of the string and write to the file
    string line,username;
    // while there is a line to read from
    while (getline(inFile,line)){
        // parse the line to get the the username
        istringstream iss(line);
        getline(iss,username,':'); // get the username (first field)
        if (username == myUserName)
            line = line + ":" + personName; // appened the personname to the end of my follow list
        cout << "Writing the line in the follow function: "<<line<<endl;
        outFile << line<<endl; // write to the file
    }
    inFile.close();
    outFile.close();
    // now delete the old file (friends.txt) & rename the temp file to friends.txt
    if (remove(inputFile) != 0){ // delete the old file{
        cout << "Error deleting the old file in follow\n";
        return "error";
    }
    else{
        if (rename (outputFile, inputFile) != 0){
            cout << "Error renaming the temp file in follow\n";
            return "error";
        }
    }

    return "success";
}


// This method searches for a particluar person's tweet
string searchPersonTweet(const string& personName){
    // call the getTweet function on this person
    try{
     
        return getTweets(personName);
    }
    catch(...){
        return "error";
    }
}



