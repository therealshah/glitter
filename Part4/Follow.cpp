#include "Follow.h"
#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>


using namespace std;


/*
-- This method gets all the people i am following
-- Will return an error code if an error occurs otherwise it will return success with the user friends
*/
string getFollowing(const std::string& username){
    
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
        //cout << "Writing the line in the follow function: "<<line<<endl;
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