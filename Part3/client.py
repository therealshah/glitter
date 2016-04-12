from flask import Flask,render_template, request, redirect,url_for, session
import os, socket
from datetime import datetime
app = Flask(__name__)

users = {} # this holds all the users Stored as {userid, {username,password}}

userTweets = {} # holds all the tweets of all the users
listStuff = [] # holds this users tweets
userFriends = {} # holds all the friends of the user


#172.16.30.242:59284
host = "127.0.0.1"
port = 13002
#host = "pdc-amd01.poly.edu" # Where do you want to connect
# host = "pdc-amd01.poly.edu"
# port = 63099 


'''----------------------------------------------------------------------

  - Everything below is the main login page. It allows the user to:
    -- LogIn/Create Account
    -- When the user First logs in, it reads all the data from the harddrive
    -- When the user logs out, it writes all the data back to the file


---------------------------------------------------------------------- '''

#This is the login page
@app.route('/', methods = ['post','get'])
def login_page():

  # ck if the user is in the session
  if 'userName' in session:
    return redirect(url_for('homePage')) # simply just redirect to the homepage of the user
  
  # If the user sends back a request( either log in or create a new account)
  if request.method == 'POST':
    # ck which type of response it was
    if request.form["submit"] == "Sign In": # user is trying to log into his/her account
      #print request.form["user_name"] #print to the console for debugging purposes
      #communicate with the server, return id
      serveroutput = servercomm("signin"+":"+request.form["user_name"]+":"+request.form["password"])
      # ck the credentials
      if serveroutput[0] == "invalid":
        return render_template("login.html", responsetext="You entered a invalid username/password")
      elif serveroutput [0] == "success":
        session['userName'] = request.form["user_name"] # basically a dictionary
        return redirect(url_for("homePage"))

    elif request.form["submit"] == "create_account":
      return redirect(url_for("create"))  # redirect to the users page
  return render_template("login.html")

# This is the create page
# Redirects here from the index page (AKA login page)
# If the user is already logged in, then just redirect to home_page
@app.route('/create/', methods = ['post', 'get'])
def create():
   # ck if the user is in the session
  if 'userName' in session:
    return redirect(url_for('homePage')) # simply just redirect to the homepage of the user

  if request.method == 'POST':
    # now get the name/username & password to create the account
    name = request.form['name'].strip('/')  # get the name
    username = request.form['user_name'].strip('/') # get the username
    password = request.form['password'].strip('/') #get the password of the user
    # note we disallow usernames,names, pass to contain ':'!
    if (':' in name) or (':' in username) or (':' in password):
      return render_template("create.html", responsetext = "Your fields contain colon (:) which are prohibited :(. Please correct the error and try again") # User entered invalid data
    else:
      # connect to socket, return information
      serveroutput = servercomm("create"+":"+username+":"+name+":"+password) # open the socket and try to account
      if serveroutput[0] == 'exists': #username already exists
        return render_template("create.html", responsetext = "User Name already taken :(")
      elif serveroutput[0] == 'newaccount':
        session['userName'] = request.form['user_name']
        return redirect(url_for("login_page"))
  else:
    return render_template("create.html")

# Log the user out and write all the data to the file
@app.route('/logout/' )
def logout():
  session.pop('userName',None) # Pop this persons name out of the saved session
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
    username = session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))

  # if we made it here, then we have a user already logged in. So get that user 
                             #get this users tweets
  if request.method == 'POST':
    if request.form['submit'] == 'tweet': # if this is a tweet request
      #Send the server the new tweet, get a list of all tweets in return
      userTweet = request.form['tweet'].strip('/')
      if ':' in userTweet: # we are not allowing the user to enter a : in the tweet
        return render_template("homepage.html",error = "You can't have a colon in your tweet. Please try again with a valid tweet or click homepage");
      elif not userTweet: # its empty
        return render_template("homepage.html",error = "You can't have a empty tweet. Please try again with a valid tweet or click the homepage");
      else:
        time = datetime.now().strftime('%Y-%m-%d %H %M %S') # get the time of when this tweet was sent
        listStuff = servercomm("tweet:"+session['userName']+":"+request.form['tweet'] + ":" +time) 
        newList = []
        if listStuff[0] == 'success': # if successfull
          listStuff.pop(0); # pop the status code
          listStuff.pop(0); # pop my username
          # loop through the list and make a pair for each datetime and tweet
          # Stored in the following way: tweet:datetime
          index = 0 # used to loop through python list
          #print 'debugging'
          while index < len(listStuff):
            mssg = listStuff[index]
            index+=1
            time = datetime.strptime(listStuff[index], "%Y-%m-%d %H %M %S")
            index += 1
            # add it to the list
            # print mssg
            # print time
            newList.append((time,mssg))
          newList = newList[::-1] # reverse the list
          #listStuff = listStuff[::-1]# reverse the list to put the latest times in the front
          return render_template("homepage.html", username = username, messages = newList)
        else:
          return render_template("homepage.html",error = "There was an error getting the tweets")
    elif request.form['submit'] == 'search-people': # User is trying to find other people that are on the site
      # If it's a search request, we will want to show what the user was searching for
      personName = request.form['find-person'].strip("/")
      if not personName:
        return redirect(url_for('homePage')) # just redirect to homepage
      else:
        return redirect(url_for('searchPeople',personName = personName)) # pass the persons name over to the function
    elif request.form['submit'] == 'search-tweet': #If the user is trying to find all the tweets of a particular person
      findPersonTweet = request.form['find-tweet'].strip("/") # get the userinput ( which is the person we wanna find) and strip the extra '/' which comes with the input
      if findPersonTweet: #make sure the request wasn't an empty request
        return redirect(url_for('searchPersonTweet',findPersonTweet = findPersonTweet)) # Redirect to this function and pass in findPersonTweet as a param
      else: #Make  sure it's not empty
        return redirect(url_for('homePage'))

  # if we made it here, we are on the same page, simply update our feedpage
  else:
    listStuff = servercomm("gettweet:"+session['userName'])    # connect to socket, return information 
    if listStuff[0] == 'success': # if successfull
      listStuff.pop(0); # pop the status code
      listStuff.pop(0); # pop my username
      newList = []
      # loop through the list and make a pair for each datetime and tweet
      # Stored in the following way: tweet:datetime
      index = 0 # used to loop through python list
      #print 'debugging within initial'
      while index < len(listStuff):
        mssg = listStuff[index]
        index+=1
        time = datetime.strptime(listStuff[index], "%Y-%m-%d %H %M %S")
        index += 1
        print mssg
        print time
        # add it to the list

        newList.append((time,mssg))
        #print (newList)
      newList = newList[::-1] # reverse to get most recent
      #listStuff = listStuff[::-1] # reverse the list, so most recent come on top
    return render_template("homepage.html", username = username, messages = newList)

# This method will basically search for people
# We could follow people from here
@app.route('/searchPeople/', methods = ['post','get'])
def searchPeople():
  try:
    username = session['userName'] # make sure there is a username
  except KeyError:  # Otherwise redirect
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page')) 

  # We are either on the page and searching for someone else
  if request.method == "POST":
    if request.form['submit'] == 'search-people': # we are searching for a friend
      personName = request.form['person-name'].strip("/")
      return redirect(url_for('searchPeople',personName = personName))
    elif request.form['submit'] == 'follow': # we are going to follow this person
      followPerson = request.form['hidden'].strip("/") # get the persons name
      statusCode = servercomm("follow:" + username + ":"+ followPerson)
      #print "testing" + statusCode[0]
      if statusCode[0] == 'success':  # it was a success
        return render_template("displayPeople.html", result = "Successfully followed " + followPerson + ":D" )
      else: # it wasn't a success
        return render_template("displayPeople.html",result = "There was an issue with following " + followPerson + ":(")

  else: # Or we were re-directed from the home-page, in this case, get the userinput that we passed in
    personName = request.args.get('personName')

  # now send the request over to c++ to find all people matching my name
  # only do it if the personName field isn't blank
  if personName:
    peopleList = servercomm("searchPeople:" + username + ":" + personName) # pass in my id so we don't return ourselves
    if peopleList[0] == 'success':
      peopleList.pop(0) # pop the error code
      #print peopleList
      return render_template("displayPeople.html", peopleList = peopleList)
    else:
      return render_template("displayPeople.html", error = "There was an error :(. Please try again later")
  else: #just return the same page
    return render_template("displayPeople.html")



# This method basically gets all the tweets of a particular user ( only allow search by username)
# Note we allow to search for anyone's tweets ( not just the people we are following)
@app.route('/searchPersonTweet/',methods = ['post','get'])
def searchPersonTweet():
  try:
    username = session['userName']
  except KeyError: # make sure the user is logged in
    return redirect(url_for('login_page'))# redirect to the login page

  # if we are already on the page and the user wants to search for something else
  if request.method == "POST":
    findPersonTweet = request.form['find-tweet'].strip("/") # get the persons input
    return redirect(url_for('searchPersonTweet',findPersonTweet = findPersonTweet)) # redirect to change the name in the url
  else: # else it was a userinput from the homepage
    findPersonTweet = request.args.get('findPersonTweet')
    # if this is empty, dedirect the page
    if not findPersonTweet:
      return render_template("displayTweets.html")
  # Open the socket and get the search for the peeps
  tweetList = servercomm("searchPersonTweet:" + findPersonTweet) # note we could also search for our own tweets
  #print tweetList
  if tweetList[0] == 'success': # we were sucessful
    tweetList.pop(0) # pop the status code
    #print 'debugging tweetList',tweetList
    personName = tweetList.pop(0) # get the persons username [ note it's stored as username:tweets]
    if tweetList: # make sure the list isn't empty
      newList = []
      # loop through the list and make a pair for each datetime and tweet
      # Stored in the following way: tweet:datetime
      index = 0 # used to loop through python list
      #print 'debugging within initial'
      while index < len(tweetList):
        mssg = tweetList[index]
        index+=1
        time = datetime.strptime(tweetList[index], "%Y-%m-%d %H %M %S")
        index += 1
        newList.append((time,mssg))# add it to the list
        #print (newList)
      newList = newList[::-1] # reverse to get most recent
      return render_template("displayTweets.html", message = "Displaying " + personName + " Tweets!", tweetList = newList)
    else:
      return render_template("displayTweets.html", message = personName + " hasn't made any tweets yet! You should them to post some now!")
  elif tweetList[0] == 'notfound': # this user doesn't exist
    tweetList.pop() # pop the status code
    return render_template("displayTweets.html", error =  findPersonTweet + " doesn't exist :(") # the user doesn't exist
  else:
    return render_template("displayTweets.html", error = "There was an error searching for " + findPersonTweet + " tweets. :(")



# This method finds all of my friends and returns them. I could also unfollow friends from this function
# We could also search for a particular person you are following
@app.route('/following/', methods = ['post','get'])
def displayMyFollowing():
  username = session['userName'] # get my username
  if request.method == 'POST':     # we are unfollowing a person
    if request.form['submit'] == 'unfollow': # if it was a unfollow request
      personName = request.form['hidden'].strip('/') # get the persons name
      followList = servercomm('unfollow:' + username + ":" + personName)  # Unfollow the friend and get the new list back
      #print 'Debugging', followList
      if (followList[0] == "success"): # If it was successful
        followList.pop(0) # popping status code
        followList.pop(0) # popping my userId
        return render_template("following.html",followList = followList)
      else:
        return render_template("following.html", error = "There was an error search for " + personName + " :(")
    else: # This request is to search for a person in ur followlist
      personName = request.form['search-name'].strip('/'); # get the user input and strip the extra character that is also added
      followList = servercomm('searchFollowList:' + username+":" + personName) 
      if (followList[0] == 'success'): # if it was successful
        followList.pop(0) # popping status code
        return render_template("following.html", followList= followList)
      else:
        return render_template("following.html", error = "There was an error search for " + personName + " :(")
  else:
    followList = servercomm('getFollowing:' + username ) # ask the server to send over all of my friends
    #print followList
    if (followList[0] == "success"):
      # meaning we are good
      # we don't care for the status code nor the userid, so remove the first two
      followList.pop(0) # popping status code
      followList.pop(0) # popping my userId
      #print followList
      return render_template("following.html", followList = followList)
    else:
      return render_template("following.html", error = "There was an error displaying your followers :(")




def checkLogIn():
  #Check if the user is logged in. If not, redirect.
  try:
    session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))

# This method basically opens the socket, sends a request and the input over
# The server handles the request and sends back the code
# The information is returned as: StatusCode:UserID:STUFF(could be friend list,tweets etc)
def servercomm(input):
  print 'yolo'
  try:
    remote_ip = socket.gethostbyname( host )
 
  except socket.gaierror:
      #could not resolve
      print 'Hostname could not be resolved. Exiting'
      sys.exit()
       
  print 'Ip address of ' + host + ' is ' + remote_ip
  s = socket.socket() # Create socket object
  s.connect((host, port))
  #print "the input:" + input
  s.send(input)
  serveroutput = (s.recv(1024)).split(":")
  #print "IN server"
  # print serveroutput
  s.close
  return serveroutput

@app.route('/delete', methods = ['post','get'])
def deleteAccount():
  global users
  #Check if the user is logged in. If not, redirect.
  try:
    session['userName']
  except KeyError:
    print("Redirect, Not Logged In")
    return redirect(url_for('login_page'))
  servercomm("delete:"+session['userName'])  #Delete all existance from the server
  return redirect(url_for('logout'))



@app.route('/force')
def force():
  session.pop('userName',None)
  listStuff[:] = [] # clears the list

  return redirect(url_for('login_page')) # redirect the user to the log in page


if __name__ == '__main__':
  app.secret_key = 'A0Zr98j/3yX R~XHH!jmN]LWX/,?RT' # this is the key used for the session
  app.run("127.0.0.2",1300,debug = True)
