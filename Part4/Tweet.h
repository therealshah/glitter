
//  Glitter
//
//  Created by Saqib Banna & Shahzaib Javed on 4/28/16.
//  Copyright (c) 2016 Learning. All rights reserved.
//

#ifndef TWEET_H
#define TWEET_H

#include <string>


    // Go through the tweet file, find the id, put the tweets
    std::string getTweets(const std::string& id,const std::string& tweetFile);
    
    void writeTweet(const std::string& id, const std::string& tweet, const std::string& timestamp,const std::string& tweetFile);

    
    
    // This method searches for a particluar person's tweet
    std::string searchPersonTweet(const std::string& personName, const std::string& tweetFile);


#endif
