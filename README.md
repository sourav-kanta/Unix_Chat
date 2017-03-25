# Unix_Chat

This is a command line chat application written for linux systems that identifies unique users via the usernames entered by them. This application uses the TCP protocol for communication

Steps to build the app :
  1. Make sure you have these packages installed <b>sqlite3</b> and <b>libsqlite3-dev</b>
  2. If not installed run this command
    **sudo apt-get install sqlite3 libsqlite3-dev**
  3. Incase of any problems in the last step consult your two best friends www.google.com and www.stackoverflow.com
  4. Once these are installed you can build the client by
      **gcc client.c -o client -lpthread**
  5. Building the server requires the following code :
      **gcc server.c -o server -lpthread -lsqlite3**
  6. Once the executables are created you can run them with
      **./server PORT_NO or ./client PORT_NO**
      
  Suppose there are two online users **SKANTA** and **UDAYAN** then messages should be of the format :
  1. For client SKANTA : **UDAYAN@hello**
  2. For client UDAYAN : **SKANTA@hi**
  
  That is the format is ***Username@Message***
