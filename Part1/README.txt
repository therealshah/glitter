************************************
PARELLEL AND DISTRIBUTED SYSTEMS 
GLITTER 
SHAHZAIB JAVED N14482418
SAQIB BANNA N15719726
************************************

FEATURES
1) STARTUP
2) CREATE ACCOUNT
3) LOG IN 
4) TWEET
5) SEARCH FRIENDS
6) DELETE


DESCRIPTIONS
1) STARTUP
	To start file run "python merged.py". 
	In browser go to "http://127.0.0.1:1300/"

2) CREATE ACCOUNT
	If account is created skip to LOG IN below.
	To create account click button "Don't have an account yet?"
	Enter a name, unique username and a password. Upon creating account you should be logged in automatically
	Account information is stored in "users.txt" in the glitter folder 

3) LOG IN 
	To log in, enter username (case sensative) and password (case sensative). No other pages can be accessed until a session is created. 
	Successful login will redirect to homepage
	Unsuccesful attempts will display error message
	Account information is stored in "users.txt" in the glitter folder 

4) TWEET
	To post a tweet, write message in the second message box and click tweet. 
	All user tweets will be displayed under recent tweets
	Tweets data is stored in "tweets.txt"

5) SEARCH FRIENDS
	To search friends type in name of user to search.
	If match found, potential users will list below. Click follow to add friends
	Search page also displays friends. Click unfollow to remove friend.
	Friends are also displayed on search page.
	Friends data is stored in "friends.txt"

6) DELETE
	To delete account, click the delete account link.
	The account will be removed from the user file and the session will end. The user id can be used again if a new account is created.
