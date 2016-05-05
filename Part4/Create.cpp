#include "Create.h"

#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>



using namespace std;
// using namespace glitter;

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