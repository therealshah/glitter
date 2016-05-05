#include "Delete.h"
#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

using namespace std;
// using namespace glitter;






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
