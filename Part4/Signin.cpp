#include "Signin.h"
#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

using namespace std;









// Function for Signing into Accounts
string signin(const string& id, const string& password, const char userFile []){
    string line, idOnFile, nameOnFile, passOnFile;
    ifstream userfile(userFile);
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

