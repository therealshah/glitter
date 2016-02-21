import re

dict = {} # dictionary for storing the result


def main ():
	global dict # modufy the global dictionary
	print "Testing for dictionaries ",not bool(dict)
	file = open('users.txt','r+') # open file for appending
	for line in file:
		credentials = line.split(':')
		s = 0
		while s < len(credentials):
			name = credentials[s]
			s+=1
			user_name = credentials[s]
			s+=1
			password=credentials[s]
			s+=1
			pair = (name,password)
			dict[user_name] = pair



	# looping through and printing the values of the dictionaries
	for key,value in dict.iteritems():
		(name,password) = value
		print key + " " + (name) + " " +(password)


	# testing purposes
	user_name = 't'
	password = 'yoyoy'
	name = 'testing'
	if user_name in dict:
		print "oops, user_name taken"
	else:
		dict[user_name] = (name,password)
		(name,password) = dict[user_name]
		print (dict[user_name])," ",(name), (password)

	print "Testing after inserting elements ",bool(dict)



	file.close() # close the file

def test():
	s1 = "yoyo"
	s2 = "yoyo"
	print s1 == s2





test()