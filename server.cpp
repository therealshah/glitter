//
//  main.cpp
//  test
//
//  Created by Saqib Banna on 3/16/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//

#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <ostream>
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

using namespace std;


#define	MAXLINE		4096	// max text line length
#define	BUFFSIZE	8192    // buffer size for reads and writes
#define  SA struct sockaddr
#define	LISTENQ		1024	// 2nd argument to listen()
#define PORT_NUM        13002

string thefunction(char theinput[MAXLINE]);
string create(const string& id,const string& username, const string& password);
string signin(const string& id, const string& password);
void readFriends(unordered_map<string,vector<string>>& map);
void writeFriends(const unordered_map<string,vector<string>>& map);
string getFollowing(const string& username);
string unfollow(const string& username,const string& friendname);

//class theuser{
//private:
//    string name,id,password;
//    vector<string> tweets;
//    
//public:
//    string getname(){return name;}
//};

int main() {

    int			listenfd, connfd;  // Unix file descriptors
    struct sockaddr_in	servaddr;          // Note C use of struct
    char		buff[MAXLINE];
    time_t		ticks;
    
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
    
    
    for ( ; ; ) {
        // 5. Block until someone connects.
        //    We could provide a sockaddr if we wanted to know details of whom
        //    we are talking to.
        //    Last arg is where to put the size of the sockaddr if
        //    we asked for one
        fprintf(stderr, "Ready to connect.\n");
        if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
            perror("accept failed");
            exit(4);
        }
        fprintf(stderr, "Connected\n");
        
        // We had a connection.  Do whatever our task is.
        ticks = time(NULL);
        // snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        char thingy[MAXLINE];
        read(connfd,thingy,MAXLINE);
        string theoutput = thefunction(thingy);
        snprintf(buff, sizeof(buff), "%s", theoutput.c_str());
        printf(buff);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }
        
        // 6. Close the connection with the current client and go back
        //    for another.
        close(connfd);
    }
}

//thetypes of actions are create,signin,tweet,search,delete,
string thefunction(char theinput[MAXLINE]){
    string thefunc;
    istringstream iss(theinput);
    getline(iss,thefunc,':');
    
    if(thefunc == "create"){
        string id, name, password;
        getline(iss,id,':');
        getline(iss,name,':');
        getline(iss,password,':');
        return create(id,name,password);
        
    } else if(thefunc == "signin"){
        string id, name, password;
        getline(iss,id,':');
        getline(iss,password,':');
        name = signin(id,password);
        
    } 
    else if(thefunc == "tweet"){
        
    } 
    else if(thefunc == "search"){
        
    } 
    else if(thefunc == "delete"){
        
    } 

    else if (thefunc == "getFollowing"){
        // get all of my friends
        string username; // used to hold the username
        getline(iss,username,':'); // get the username
        return getFollowing(username); // gets all the people i am following
    }
    else if (thefunc == "unfollow"){
        // remove this from my follow list
        string username,friendname; // remove the friend from my friends list
        getline(iss,username,':'); // my name
        getline(iss,friendname,':'); // friendname
        return unfollow(username,friendname);
    }


    else {
        return "hi";
    }
    return "Should not reach here";
};

string create(const string& id, const string& username, const string& password){
    string line, token;
    fstream idfile("users.txt");
    if(!idfile.is_open()){cout<< "FILE ISNT OPEN \n";}
    
    // Does the ID already exist? If so, leave nested loop.
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
    //cout << "reached afterread \n";
    
    if (idexists == 1){
        return "exists";
    } else {
        // ID doesn't exist, add it.
        //cout << "reached afterread2 \n";
        ofstream myfile;
        myfile.open ("users.txt",ios::app);
        myfile << id << ":" << username << ":" << password << "\n";
        myfile.close();
        return "hi";
    }
};

string signin(const string& id, const string& password){
    string line, token, nameOnFile, passOnFile;
    ifstream userfile("users.txt");
    if(!userfile.is_open()){cout<< "FILE ISNT OPEN \n";}
    
    // Does the ID already exist? If so, leave nested loop.
    bool idpassvalid = 0;
    while(getline(userfile, line)){
        cout << line << endl;
        istringstream spilt(line);
        token = "";
        while(getline(spilt, token, ':')){
            if(id==token){
                getline(spilt, nameOnFile, ':');
                getline(spilt, passOnFile, ':');
                if(passOnFile==password){
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
        return nameOnFile;
    }
};

// string getTweets(string id){
//     string line, token, nameOnFile, passOnFile;
//     ifstream userfile("tweets.txt");
//     vector<string> tweetholder;
//     if(!userfile.is_open()){cout<< "FILE ISNT OPEN \n";}
    
//     while(getline(userfile, line)){
//         cout << line << endl;
//         istringstream spilt(line);
//         token = "";
//         while(getline(spilt, token, ':')){
//             if(id==token){
//                 getline(spilt, nameOnFile, ':');
//                 getline(spilt, passOnFile, ':');
//                 if(passOnFile==password){
//                     idpassvalid = 1;
//                 }
//                 goto afterreading;
//             }
//             break;
//         }
//     }
//     afterreading:
//     userfile.close();
//     if (idpassvalid == 0){
//         return "invalid";
//     } else {
//         return nameOnFile;
//     }
// };



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
    string line;
    string user;
    while (getline(ifs,line)){
        // while we have a line to read
        // ck the user
        int pos = line.find_first_of(':');
        user = line.substr(0,pos); // get the user
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
    -- This method accepts a username and a friendname
    -- basically removes the friend from the followlist of username

*/
string unfollow(const string& username,const string& friendname){

    // first open the friends txt and read all the friends
 
    // we will read the names in a hashmap and update the friends list and then write back to the file
    unordered_map<string,vector<string>> map;
    readFriends(map);
    // now find the usernames friends vector
    // note we retrieve the vector by reference
    vector<string>& friends = map[username]; 
    // erase the friend

    auto it = find(friends.begin(),friends.end(),friendname);
    friends.erase(it); // erase this value
    // now write the values back
    writeFriends(map);
    // return my new list
    return getFollowing(username); // get all the peeps im following

}


// opens the file and reads it into the map
void readFriends(unordered_map<string,vector<string>>& map){
       ifstream ifs("friends.txt");// open the file for reading
       string line;
       // keep reading while there is a line to read
       while (getline(ifs,line)){
        // now we will split the string using :
        string token;
        vector<string> friends;
        string name;
        istringstream iss(line); // split the name using :
        getline(iss,name,':'); // get the username since thats the first for thing after the colon
        // now keep reading the friend
        while (getline(iss,token,':'))
            friends.push_back(token);

        // now insert it into the haspmap
        map[name] = friends;
       }
       ifs.close();
}

// writes the friends back
void writeFriends(const unordered_map<string,vector<string>>& map){

    ofstream ifs; // overwrite the file
    ifs.open("friends.txt",ios::out);
    if (!ifs){
        cout <<"couldn't open friends file for writing\n";
    }
    auto it = map.begin();
    while (it != map.end()){
        // loop through the hashmap
        ifs<<it->first;
        vector<string> friends = it->second;
        for (const string& s: friends)
            ifs<<":"<<s;
        ifs<<endl;
        ++it;
    }

    ifs.close();

}




























