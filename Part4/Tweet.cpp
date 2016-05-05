#include "Tweet.h"
#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

using namespace std;
// using namespace g


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

