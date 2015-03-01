#import urllib
import urllib, json
import time

def send_email():
    import smtplib

    gmail_user = "pythonicwidd@gmail.com"
    gmail_pwd = "AppleOrOrange?"
    FROM = 'pythonicwidd@gmail.com'
    TO = ['widder2@illinois.edu'] #must be a list
    SUBJECT = "Laundry notification"
    TEXT = "I would like to inform you that your laundry is finished."

    # Prepare actual message
    message = """\From: %s\nTo: %s\nSubject: %s\n\n%s
    """ % (FROM, ", ".join(TO), SUBJECT, TEXT)
    try:
        #server = smtplib.SMTP(SERVER) 
        server = smtplib.SMTP("smtp.gmail.com", 587) #or port 465 doesn't seem to work!
        server.ehlo()
        server.starttls()
        server.login(gmail_user, gmail_pwd)
        server.sendmail(FROM, TO, message)
        #server.quit()
        server.close()
        print 'successfully sent the mail'
    except:
        print "failed to send mail"

url = "https://api.spark.io/v1/devices/54ff76066667515115461467/accel?access_token=d1f90f338d98d15a0b824250b5d959bacd8be2a6"
response = urllib.urlopen(url);
data = json.loads(response.read())
print "Current vibrational data is: " + str(data['result'])
tolerance = 2

active = True
prevVib = data['result']
#currVib = data['result']
stdVib = 102
send = False
i = 0
while(1):
	response = urllib.urlopen(url);
	data = json.loads(response.read())
	vib = data['result']
	print "Current vibrational data is: " + str(vib)

	if abs(vib - stdVib) >= tolerance and not active:
		active = True
		print "Activating laundry detector"
		i = 0
	elif abs(vib - stdVib) >= tolerance:
		i = 0

	if abs(vib - stdVib) < tolerance:
		i += 1
		print str(i) + " iterations of a quiet laundry machine"
	if i > 5 and active:
		send = True

	if send:
		send_email()
		send = False
		i = 0
		active = False
		print "Deactivating laundry detector"
	time.sleep(2)
	














