from flask import Flask,render_template, request, redirect,url_for, session
import os
app = Flask(__name__)

users = {} # this holds all the users Stored as {userid, {username,password}}

userTweets = {} # holds all the tweets of all the users
listStuff = [] # holds this users tweets
userFriends = {} # holds all the friends of the user

'''----------------------------------------------------------------------

  - Everything below is the main login page. It allows the user to:
    -- LogIn/Create Account
    -- When the user First logs in, it reads all the data from the harddrive
    -- When the user logs out, it writes all the data back to the file


---------------------------------------------------------------------- '''

#This is the login page
@app.route('/', methods = ['post','get'])
def login_page():
  global users
  global userFriends
  #check to see if the users dictionary is empty, if it is read it in from the file
  if not bool(users): # if its empty, read it in from the file
    print "Reading in from the file"
    readUsers(users)
  if not bool(userTweets): # if we haven't read in the user tweets, then read them in
    readTweets(userTweets)
  if not bool(userFriends): 
    userFriends = readFriends(userFriends)

  # ck if the user is in the session
  if 'userName' in session:
    print "IN HERE "
    return redirect(url_for('homePage')) # simply just redirect to the homepage of the user
  # If the user sends back a request( either log in or create a new account)
  if request.method == 'POST':
    # ck which type of response it was
    if request.form["submit"] == "Sign In": # user is trying to log into his/her account
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
      session['userName'] = userName
      # listStuff.insert(0,"")
      # followFriend("") 
      return redirect(url_for("login_page"))

  return render_template("create.html")

# read the users from the file
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
    username = string[0].strip("\n"); #always strip this
    tempList = [] # holds the tweets
    while i < len(string):
      tempList.append(string[i].strip("\n"))
      i+=1
    # add it to the list
    tweets[username] = tempList
  print 'tweets'
  print (tweets)
  file.close()


#this method basically reads the friends of the users from the file
def readFriends(friends):
  file = open("friends.txt","r")
  for line in file:
    string  = line.split(':') # split the tweets based on the colon (:) ( will have multiple tweets per user)
    i = 1 # zeroth index is the userId 
    username = string[0].strip("\n") # always strip this
    tempList = [] # holds the tweets
    while i < len(string):
      tempList.append(string[i].strip("\n"))
      i+=1
    # add it to the list
    friends[username] = tempList
  # for line in file:
  #   string  = line.split(':') # split the tweets based on the colon (:) ( will have multiple tweets per user)
  #   if string[0] == session['userName']:
  #     print("I AM HERE")
  #     i = 1 # zeroth index is the userId 
  #     tempList = [] # holds the tweets
  #     while i < len(string):
  #       tempList.append(string[i].strip("\n"))
  #       i+=1
  #     # add it to the list
  #     return tempList
  #     print(tempList)
  #     break
  file.close()
  return friends


#When we successfully create a user, insert into the dictionary and write to the file
def writeKey(users,name,userName,password):
  users[userName] = (name,password) #insert into the dictionary
  file = open("users.txt","a") #write to the file
  line = name + ":"+userName+":"+password+"\n"
  file.write(line)
  file.close()
  # also make a blank entry for the user tweets and friend list
  userTweets[userName] = []
  userFriends[userName] = []

#Verify whether the user entered the user_id and password correctly
def checkCredentials(userName,typedPass):
  global users 
  if users.has_key(userName):
    (name,userPass) = users[userName]
    return (typedPass == userPass)
  else:
    return False

# Log the user out and write all the data to the file
@app.route('/logout/' )
def logout():
  global listStuff
  # before we logout, we will store all the tweets on the file!

  #Save the tweets
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

  file = open("friends.txt",'w') #Save all the friends
  for key,friends in userFriends.iteritems():
    #write the userId
    file.write(key);
    #now loop through all the friends and write them to the file
    for friend in friends:
      file.write(":" + friend) 
    file.write("\n") #write the new line character for the next user

  file.close() #close the file


  session.pop('userName',None)
  listStuff[:] = [] # clears the list
  return redirect(url_for('login_page')) # redirect the user to the log in page


'''------------------------------------------------------------------
  -- Everything below is:
    -- Homepage which allows the user to view his/her tweets
    -- Post tweets
    -- Search for friends
    -- Search for people
    -- See friends list


---------------------------------------------------------------------'''

# This is the main homepage that allows for all the features discussed above
@app.route('/homepage/', methods = ['post','get'])
def homePage():
  #Check if the user is logged in. If not, redirect.
  try:
    session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))

  # if we made it here, then we have a user already logged in. So get that user 
  username = session['userName'] # get the username from the cache



  listStuff = userTweets[username] #get this users tweets

  # if the user is trying to log out, then log out
  if request.method == 'POST':

    if request.form['submit'] == 'tweet': # if this is a tweet request
      # add this to the dictionary of the user. Along with the time stamp
      #userList = userTweets[username] # get the userTweets
      # userList is basically a list that contains the tweets
      #listStuff.append(request.form['tweet']) # add it to the list we are using currently as well
      listStuff.insert(0,request.form['tweet'])
    elif request.form['submit'] == 'search':
      # If it's a search request, we will want to show what the user was searching for
      userinput = request.form['search']
      print 'testing ' + userinput
      if userinput:
        return redirect(url_for('search',userinput = userinput))
     


  return render_template("homepage.html", username = username, messages = listStuff)


@app.route('/search/<userinput>', methods = ['post','get'])
def search(userinput):
  #Check if the user is logged in. If not, redirect.
  try:
    session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))

  # Get all the tweets of this person if they are a friend
  # Get all the friends that match up with this name
  # Get all the potential friends that match up with the search request
  if request.method == 'POST':
    if request.form['submit'] == 'search':
      userinput = request.form['search']
    if request.form['submit'] == 'follow':
      #   #want to follow this dude
      # print 'IN IF STATEMENT'
      friend = request.form['hidden']           
      print " printing friend " + friend 
      followFriend(friend) # follow the friend
    elif request.form['submit'] == 'unfollow': # unfollow the person
      friend = request.form['hidden'] # friend to unfollow
      unFollowFriend(friend)


  
  potentialFriends = [] # find all friends that match up with this name
  tweets = [] # get all the tweets that match with this search
  myFriends = [] # find all of my friends that match with this name
  # if the input isnt empty
  if userinput.strip("/"):
    print "printing user inpit " + userinput
    findPotentialFriends(potentialFriends,userinput)
    findTweets(tweets,userinput)
    myFriends = findMyFriends()
  #print 'Printing user input ' + userinput
  print 'Printing all of my friends  '
  print (tweets)
  return render_template("search.html", potentialFriends = potentialFriends, tweets = tweets, myFriends = myFriends)


# Finds all the friends based on the name provided
def findPotentialFriends(potentialFriends,userinput):
  # loop through all the users and find all of them that match this name
  friendList = userFriends[session['userName']] # get this persons friends
  for key,value in users.iteritems():
    (username,password) = value #split the pair
    if (userinput == username and not(key in friendList)):
      # we have found a match
      potentialFriends.append((key,username))
  # debugging purposes
  print(potentialFriends)
 

def findTweets(tweets,userinput):
  # if it's a friend, find all the tweets of this person
  friendList = findMyFriends()
  if userinput in friendList: # checking if this search is a friend
    #if yes, get all the tweets of this person
    friendTweets = userTweets[userinput]
    for tweet in friendTweets:
      tweets.append((userinput,tweet))# add this tweet to the tweet list

  print "printing friends "
  print (friendList)
  #also check if any of my friends tweets matched the search
  for friend in friendList:
    try:
      userTweets[friend]
    except:
      break
    friendTweets = userTweets[friend]
    for tweet in friendTweets:
        tweets.append((userinput,tweet)) # add this tweet to the tweet list 
  print "printing all tweets "
  print (tweets)


def findMyFriends():
  # find all of my friends and store them in the list
  username = session['userName'] # get my username
  return userFriends[username]

# Follow the person
def followFriend(friend):
  global userFriends
  print 'in follow friends'
  username = session['userName'] # get my username
  friendList = userFriends[username]
  # just append it to the back of the list
  friendList.append(friend.strip("/")) # html sends out a slash
  print 'friendsList'
  print(friendList)

#Unfollow the person
def unFollowFriend(friend):
  global userFriends
  username = session['userName'] #get my username
  friendList = userFriends[username] # get all of my friends
  friendList.remove(friend.strip("/")) #remove this friend. The html sends an extra / over

def checkLogIn():
  #Check if the user is logged in. If not, redirect.
  try:
    session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))

@app.route('/delete', methods = ['post','get'])
def deleteAccount():
  global users
  #Check if the user is logged in. If not, redirect.
  try:
    session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))

  username = session['userName']
  userTweets.pop(username, None)
  users.pop(username, None)
  listStuff = []
  userFriends.pop(username, None)

  # also find all the people that were following me and delete myself from the list
  for User,friendList in userFriends.iteritems():
    friendList.remove(username) # remove this guy from everyones list


  file = open("users.txt",'w') # we will overwrite the existing file with the new one
  # loop through the list
  for key,user in users.iteritems():
    #write the userId
    file.write(key +":" + user[0] + ":" + user[1] + "\n");
    #now loop through all the tweets and write them to the file
    # for user,password in users:
    #   file.write(":" + user + ":" + password) 
    # file.write("\n") #write the new line character for the next user

  file.close() #close the file
  return redirect(url_for('logout'))




@app.route('/force')
def force():
  session.pop('userName',None)
  listStuff[:] = [] # clears the list

  return redirect(url_for('login_page')) # redirect the user to the log in page



if __name__ == '__main__':
  app.secret_key = 'A0Zr98j/3yX R~XHH!jmN]LWX/,?RT' # this is the key used for the session
  app.run("127.0.0.1",1300,debug = True)
