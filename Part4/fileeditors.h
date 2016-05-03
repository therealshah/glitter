//
//  fileeditors.h
//  Glitter
//
//  Created by Saqib Banna on 4/28/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//

#ifndef __Glitter__fileeditors__
#define __Glitter__fileeditors__

#include <stdio.h>
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
// bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <algorithm>
#include <queue>
#include <utility>

namespace glitter{
    // Function for Creating new accounts
    std::string create(const std::string& id, const std::string& username, const std::string& password);
    
    // Function for Signing into Accounts
    std::string signin(const std::string& id, const std::string& password);
    
    // Go through the tweet file, find the id, put the tweets
    std::string getTweets(const std::string& id);
    
    void writeTweet(const std::string& id, const std::string& tweet, const std::string& timestamp);
    
    // This method deletes the user from all files by calling other methods
    std::string deleteAccount(const std::string& id);
    
    // This method removes any line that start with the client ID in any file
    bool removeIDfromfile(const std::string& clientID, const char theFile[]);
    
    // This method removes your ID from everyone following you
    // The file is copied over without any instance of the clientID in parameter
    // A 1 is returned if the process is successful
    bool removeIdFromYourFollowers(const std::string& clientID);
    
    
    /*
     -- This method gets all the people i am following
     -- Will return an error code if an error occurs otherwise it will return success with the user friends
     */
    std::string getFollowing(const std::string& username);
    
    /*
     -- This has the same task as the getFollowing method but only returns
     -- the people that the same username
     --  makes use of the getFollowing method
     
     */
    std::string getFollowing(const std::string& username, const std::string& personName);
    
    /*
     -- This method accepts a username and a friendname
     -- basically removes the friend from the followlist of username
     
     */
    std::string unfollow(const std::string& username,const std::string& personName);
    
    
    
    // Finds all the people that are friends of the username
    std::string findPeople(const std::string& myUserName, const std::string& personName);
    
    
    // This method will bascically add the person to the friend vector
    std::string follow(const std::string& myUserName, const std::string& personName);
    
    
    // This method searches for a particluar person's tweet
    std::string searchPersonTweet(const std::string& personName);

}



#endif /* defined(__Glitter__fileeditors__) */
