************************************************************************
PARELLEL AND DISTRIBUTED SYSTEMS 
GLITTER 
SHAHZAIB JAVED N14482418
SAQIB BANNA N15719726
************************************************************************

TABLE OF CONTENTS
1) FEATURES (UPDATED IN PART 4)
2) CLIENT SERVER COMMUNICATION 
3) FILES
4) MULTITHREADING
5) SERVER REPLICATION (UPDATED IN PART 4)
6) SERVER RECOVERY (UPDATED IN PART 4)

************************************************************************

FEATURES

1) STARTUP (UPDATED IN PART 4)
2) CREATE ACCOUNT
3) LOG IN 
4) TWEET
5) SEARCH FRIENDS
6) SEARCH TWEETS
7) DELETE
8) FOLLOWING 


DESCRIPTIONS
1) STARTUP
	To start servers run the makefile using the command "make"
	Run ./server1 and ./server2

	To start flask run "python client.py". 
	In browser go to "http://127.0.0.1:1300/"
	To start C++ server run server.cpp

	WARNING: When starting, you have to sign in one time to any account before creating an account. Flask is not able to access the global variable it needs to for some reason and will crash.

	NOTE: This project assumes that flask does not crash as said by Robert the TA. If flask crashes unexpectedly the sequence numbers for the files will be off. To provide Robert the TA an easy solution to this, go to /Part4/goodfiles and copy and replace the files from there into the /Part4 folder. Cheers!


2) CREATE ACCOUNT
	If account is created skip to LOG IN below.
	To create account click button "Don't have an account yet?"
	Enter a name, unique username and a password. Upon creating account you should be logged in automatically
	Account information is stored in "users.txt" in the glitter folder
	Every time a new account is created a the “users.txt” file is appended 

3) LOG IN 
	To log in, enter username (case sensitive) and password (case sensitive). No other pages can be accessed until a session is created. 
	Successful login will redirect to homepage
	Unsuccessful attempts will display error message
	Account information is stored in "users.txt" in the glitter folder 

4) TWEET
	To post a tweet, write message in the second message box and click tweet. 
	All user tweets will be displayed under recent tweets
	Tweets data is stored in "tweets.txt"
	Every time a new tweets is created a the “tweets.txt” file is overwritten

5) SEARCH FRIENDS
	To search for new friends type in name of user to search.
	If match found, potential users will list below. Click follow to add friends
	Once added friends can be found in following.
	Friends data is stored in "friends.txt"

6) SEARCH TWEETS
	To search friends type in name of user to search.
	If match found, potential tweets will list below. Click follow to add friends
	Tweet data is stored in “tweets.txt"

7) DELETE
	To delete account, click the delete account link.
	The account will be removed from the user file and the session will end. The user id can be used again if a new account is created.

8) FOLLOWING
	Friends added on search will show on the “Following” page
	Friends can be unfollowed
	Unfollowed friends can later be re-followed and searched
	Friends data is modified in “friends.txt”


************************************************************************

CLIENT SERVER COMMUNCIATION 

	The client communicates with the server by sending string.
	Strings have : that separate information
	The first part of the string contains the function that needs to operate. Then comes the information needed for the function
	The server then checks the information and returns what is needed.
	The client splits the returned string using the : to tokenize. The tokens are then presented in the output interface. 


************************************************************************

FILES 

	The files the server uses is friends.txt, users.txt, tweets.txt
	Information is stored using lines that have : to tokenize information. 
	The first part of the line is always a specific identifier.

************************************************************************

MULTITHREADING

	Multithreading is used in this program to allow consecutive reads to occur at the same time and allow the server to service multiple connections

	IMPORTANT THREADS
		MAIN THREAD: The main thread is responsible for listening to the socket and making connections to accept requests. It then pushes the request and the connection into a global queue for our bouncer thread to service. After a request is queued the main thread listens for more connections

			* The queue shared by the main thread is a critical region. A lock must be acquired to read or write into this queue. Upon completing the task the lock is released.

			* A unique lock is used to allow the bouncer thread to know when the main thread has pushed in a request.

		BOUNCER THREAD: The bouncer thread executes the manage thread function. It checks the queue for any requests and creates a thread when it is time to execute a task. The tasks in the queue are either a read function or a write function. The bouncer thread is in charge of making sure of the following 
			1) First come first serve - priority if given to requests that are at the top of the queue to ensure fairness
			2) Consecutive parellel reads - If a read is occuring and is in the critical region then all consecutive reads are allowed to access the critical region. No writes are allowed to happen until the last read is done. This is implemented by using a counter to keep track of how many reads remain in the critical region 
			3) Single writes - If a write occurs, bouncer makes sure no other reads or writes are in the critical region. Once the desired write has access to the critical region no other reads or writes are allowed to access critical region. A write lock is used to implement this

		* Locks are aquired in a specific order to prevent deadlocks. If a lock cannot be aquire it is let go and the thread will try again later. 
		* Unique locks are used to wake up sleeping threads instead of constantly waking up and going back to sleep


	READ AND WRITE MANAGEMENT
		BOUNCER THREAD: The bouncer thread holds all request IN ORDER until they have the safety to execute. Unique locks are used to wake the bouncer thread as needed. 

		READ: When reads occur a counter is used to keep track of how many reads are occuring. Each time a read executes bouncer acquires the proper locks, increments the read counter, lets go of the lock and does the read operation by launching a thread. Once the read is done, it aquires the proper locks again, decrements the counter and lets go of the lock. If the a thread decrements the counter to 0 is notifies the bouncer thread that no reads are in the critical region.

		WRITE: When a write occurs a writeishappening bool is used to keep track of the critical region. Each time a write occurs boucner acquires the proper locks, checks that the read counter is at 0, sets the writeishappening bool, lets go of the locks and does a write operation by launching a thread. Once the write is done, it aquires the proper locks again, resets the write is happening bools, lets go of the locks and notifys bouncer to continue passing in more functions. 

************************************************************************

SERVER REPLICATION
	
	ACTIVE REPLICATION: 
		FLASK: Acts as the front end server. 
			- Flask has a queue of all the server it should send the request to. Flask takes a request from the client and sends only the write requests to all the servers.  Flask waits for all the servers to respond and has a 5 second timeout. If a timeout occurs flask assumes that server is down. When sending requests to servers, flask also sends sequence numbers to the servers to make sure the servers are handling requests in the corrext order
		SERVER: Handles all read and write requests
			- Servers do not know if they are a primary or a backup server. All servers process the request and make sure they are handling sequence numbers in the proper order. If a request has a sequence number greater than the servers current sequence number + 1 then the servers assume they have crashed because they are out of date. They will then ask Flask to provide the last few requests. If a request is properly handled, flask will acknowledge that a server is up to date. Flask has a vector keeping track of what sequence number each server is on.

************************************************************************

SERVER RECOVERY:
	
	What happens when a server crashes??
		
		1) Flask is sending out a requests with sequence numbers to all servers
		2) The sequence numbers for the servers are updated
		3) Assume a server (Server 2 in this case crashes)
		4) Flask sends out requests with sequence numbers to all servers but timeouts for server 2
		5) Eventually server 2 comes back online
		6) On the next request server 2 tells flask its behind and it needs old requests
		7) Server 2 blocks all incoming requests as it uses the queue it received to catch up
		8) Meanwhile other servers as handling all the requests
		9) Once server 2 is caught up it uses its sequence number to check the requests that it blocks. It then updates as needed
		10) Server 2 is now up to date and is running as needed. 
		11) Flask checks it vector of responses every 10 requests to see if the servers are up to date. If they are flask will delete old requests from memory. 














