
//  Glitter
//
//  Created by Saqib Banna & Shahzaib Javed on 4/28/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//

#ifndef DELETE_H
#define DELETE_H

#include <string>


	// This method deletes the user from all files by calling other methods
    std::string deleteAccount(const std::string& id,const std::string& tweetsFile, const std::string& usersFile, const std::string& friendFile);
    
    // This method removes any line that start with the client ID in any file
    bool removeIDfromfile(const std::string& clientID, const char theFile[]);
    
    // This method removes your ID from everyone following you
    // The file is copied over without any instance of the clientID in parameter
    // A 1 is returned if the process is successful
    bool removeIdFromYourFollowers(const std::string& clientID,const std::string& friendsFile);

#endif
