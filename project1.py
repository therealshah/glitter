from flask import Flask, render_template,request,redirect,url_for, session
app = Flask(__name__)

users = {} # this holds all the users

userTweets = {} # holds all the tweets of all the users
listStuff = [] # holds this users tweets

@app.route('/homepage/', methods = ['post','get'])
def homePage():

	# if we made it here, then we have a user already logged in. So get that user
	username = session['userName'] # get the username from the cache
	listStuff = userTweets[username] #get this users tweets

	# if the user is trying to log out, then log out
	if request.method == 'POST':
		if request.form['submit'] == 'logout':
			print 'logging out\n'
			return logout() #log out and redirect the user to the login page

		elif request.form['submit'] == 'tweet': # if this is a tweet request
			# add this to the dictionary of the user. Along with the time stamp
			#userList = userTweets[username] # get the userTweets
			# userList is basically a list that contains the tweets
			#listStuff.append(request.form['tweet']) # add it to the list we are using currently as well
			listStuff.insert(0,request.form['tweet'])


	return render_template("homepage.html", username = username, messages = listStuff)



# This is the create page
# Redirects here from the index page (AKA login page)
@app.route('/create/', methods = ['post', 'get'])
def create():
	global users
	if request.method == 'POST':
		#now get the name/username & password to create the account
		name = request.form['name'] # get the name
		userName = request.form['user_name'] # get User name
		password = request.form['password'] # get pass
		# check to see if this user_name exists because they must be unique!
		if users.has_key(userName):
			print "error key already exists"
			return render_template("create.html", responsetext = "User Name already taken :(")
		else:
			print "Key successfully created! "
			writeKey(users,name,userName,password) # write the key to the dictionary and to the file
			return "Account created!"

	return render_template("create.html")

#This is the login page
@app.route('/', methods = ['post','get'])
def login_page():
	global users
	#check to see if the users dictionary is empty, if it is read it in from the file
	if not bool(users): # if its empty, read it in from the file
		print "Reading in from the file"
		readUsers(users)
	if not bool(userTweets): # if we haven't read in the user tweets, then read them in
		readTweets(userTweets)
	# ck if the user is in the session
	if 'userName' in session:
		print "IN HERE "
		return redirect(url_for('homePage')) # simply just redirect to the homepage of the user
	# If the user sends back a request( either log in or create a new account)
	if request.method == 'POST':
		# ck which type of response it was
		if request.form["submit"] == "log_in": # user is trying to log into his/her account
			print request.form["user_name"] #print to the console for debugging purposes
			userName = request.form["user_name"]
			password = (request.form["password"])
			# ck the credentials
			if checkCredentials(userName,password): #verification was a success
				# add it to the session, meaning we cache that the user is logged in. 
				# The user won't have to log back in
				session['userName'] = userName # basically a dictionary
				return redirect(url_for("homePage"))
			else:
				print "failed"
				return render_template("login.html", responsetext="You entered a invalid username/password")


		elif request.form["submit"] == "create_account":
			return redirect(url_for("create"))  # redirect to the users page
	return render_template("login.html")


#Basically read the users from the textfile into main memory
def readUsers(users):
	# so read the file and store all the users in the dictionary
	file = open("users.txt","r") #open file for only reading
	for line in file:
		string  = line.split(':') # split the line based on colon (:). (name userId )
		i = 0
		while i < len(string): #loop through the file and read in the users
			name = string[i]
			i+=1
			user_name = string[i]
			i+= 1
			password = string[i].strip("\n")
			i+=1
			users[user_name] = (name,password)

	print (users)
	file.close()

#this method basically reads the tweets of the users from the file
def readTweets(tweets):
	file = open("tweets.txt","r")
	for line in file:
		string  = line.split(':') # split the tweets based on the colon (:) ( will have multiple tweets per user)
		i = 1 # zeroth index is the userId 
		username = string[0]
		tempList = [] # holds the tweets
		while i < len(string):
			tempList.append(string[i].strip("\n"))
			i+=1
		# add it to the list
		tweets[username] = tempList
	print (tweets)
	file.close()



#When we successfully create a user, insert into the dictionary and write to the file
def writeKey(users,name,userName,password):
	users[userName] = (name,password) #insert into the dictionary
	file = open("users.txt","a") #write to the file
	line = name + ":"+userName+":"+password+"\n"
	file.write(line)
	file.close()
	# also make a blank entry for the user tweets
	userTweets[userName] = []

#Verify whether the user entered the user_id and password correctly
def checkCredentials(userName,typedPass):
	global users 
	if users.has_key(userName):

		(name,userPass) = users[userName]
		return (typedPass == userPass)
	else:
		return False


@app.route('logout/' )
#log the user out
def logout():
	global listStuff
	# before we logout, we will store all the tweets on the file!
	file = open("tweets.txt",'w') # we will overwrite the existing file with the new one
	# loop through the list

	for key,tweets in userTweets.iteritems():
		#write the userId
		file.write(key);
		#now loop through all the tweets and write them to the file
		for tweet in tweets:
			file.write(":" + tweet) 
		file.write("\n") #write the new line character for the next user

	file.close() #close the file

	session.pop('userName',None)
	listStuff[:] = [] # clears the list


	return redirect(url_for('login_page')) # redirect the user to the log in page










if __name__ == '__main__':
	app.secret_key = 'A0Zr98j/3yX R~XHH!jmN]LWX/,?RT' # this is the key used for the session
	app.run("127.0.0.1",1300,debug = True)

# with app.test_request_context():
# 	print url_for('login')