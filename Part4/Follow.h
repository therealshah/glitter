
//  Glitter
//
//  Created by Saqib Banna & Shahzaib Javed on 4/28/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//

#ifndef FOLLOW_H
#define FOLLOW_H

#include <string>


	 /*
     -- This method gets all the people i am following
     -- Will return an error code if an error occurs otherwise it will return success with the user friends
     */
    std::string getFollowing(const std::string& username,const char friendFile []);
    
    /*
     -- This has the same task as the getFollowing method but only returns
     -- the people that the same username
     --  makes use of the getFollowing method
     
     */
    std::string getFollowing(const std::string& username, const std::string& personName,const char friendFile []);
    
    /*
     -- This method accepts a username and a friendname
     -- basically removes the friend from the followlist of username
     
     */
    std::string unfollow(const std::string& username,const std::string& personName,const char friendFile []);
    
    
    
    // Finds all the people that are friends of the username
    std::string findPeople(const std::string& myUserName, const std::string& personName,const char userFile []);
    
    
    // This method will bascically add the person to the friend vector
    std::string follow(const std::string& myUserName, const std::string& personName,const char friendFile []);

#endif
