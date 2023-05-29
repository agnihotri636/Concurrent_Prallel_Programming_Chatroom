//Makefile
Compile all programs at once using command ‘make’

This will generate 2 files
1. chat_server
2. chat_client

To run server
$ chat_server config_server

To run client
$ chat_client config_client

//Avaialble commands for user
Login                     : login <username>
Logout                    : logout <username>
Chat with particular user : chat <@username> <message>
Broadcast                 : chat <message>

The server will display all online users.

To clean the environment run ‘make clean’. This will delete all .o chat_server and chat_client files.

Note: Please try changing port number in config_server file in case of any error(bind error).

//-------------------------------------------------------------------------------------------------------

//select and pthread
In my project, I used both pthread and select() to implement a multi-threaded client-server chatroom system. 
Pthread was used to create multiple threads on the server side to handle different clients, while select() 
was used to multiplex the I/O operations of the clients and the server.

When a client connects to the server, a new thread is created to handle that client's requests. 
The thread listens for incoming messages from the client and sends them to all other clients in the chatroom.

On the client side, select() is used to wait for input from the user or from the server. If input is available
from the user, it is read and parsed to determine the appropriate action. If input is available from the server,
it is read and displayed to the user.

Other server models that could have been used include the forking model, where a new process is created for each
client, and the event-driven model, where a single thread is used to handle I/O operations for multiple clients. 
However, I chose to use the multi-threaded model because it allows for better performance and scalability.

//-------------------------------------------------------------------------------------------------------

//Detailed explaination of usage of select

1. First, the program initializes a file descriptor set called fds_ using the FD_ZERO() and FD_SET() functions. 
   FD_ZERO() clears the set, and FD_SET() adds the file descriptor of the standard input (file descriptor 0) to 
   the set.

	// set up the file descriptor set for the select function
	FD_ZERO(&fds_);
	FD_SET(0, &fds_);

2. If the client socket is connected (i.e., client_sock is not -1), the program adds the file descriptor of the 
   socket to the set using FD_SET().

	if (set_sock() != -1)
    	FD_SET(set_sock(), &fds_);

3. The select() function is then called, passing in the maximum file descriptor (getdtablesize()) and the file
   descriptor set. The function blocks until there is input available on one of the file descriptors in the set.

   switch (select(max_fd, &fds_, NULL, NULL, NULL)) {

4. Once input is available, select() returns, and the program checks which file descriptor has input available
   using FD_ISSET().

	if (set_sock() != -1 && FD_ISSET(set_sock(), &fds_)) {
   	// there is input available on the socket, read it in and print to the console
   	...
	} else if (FD_ISSET(0, &fds_)) {
   	// there is input available from the user, read it in and parse the command
   	...
	}

5. If input is available on the socket, the program reads it in using recv(), processes it, and prints it to
   the console.

	char packet[256];
	memset(packet, 0, sizeof(packet));
	int socket_length = (int)::recv(set_sock(), packet, 256, 0);
	if (socket_length > 0) {
	   cout << packet << endl;
	}

6. If input is available from the user, the program reads it in using fgets(), parses the command, and executes
   the appropriate function.

	char packet[256];
	fgets(packet, 256, stdin);
	string comnd = packet;
	parse_command(comnd);

7. The program then loops back to step 1 and waits for input again.

The select() function is used in this program to wait for input from multiple sources (the standard input and the 
socket file descriptor), without blocking on any one of them. It allows the program to wait for input without 
wasting CPU cycles constantly polling for input on each file descriptor. Once input is available on one of the file
descriptors, the program can quickly process it and move on to the next input source. This is an efficient way to 
handle multiple input sources in a single-threaded program.

//-------------------------------------------------------------------------------------------------------

//Detailed explaination of usage of pthread

In this project, I used pthreads (POSIX threads) which are used to manage client connections in separate threads. 
When a new client connects to the server, a new thread is created to manage that client's connection. This is important
because it allows the server to handle multiple client connections simultaneously, without blocking on any one client's
input or output.

The function manage_client_thread is responsible for managing a single client connection in a separate thread. It takes
a socket descriptor as an argument, which is used to communicate with the client. The function listens for incoming
messages from the client using the recv function, and then processes those messages using the parse_command function.
If the recv function returns -1, the thread terminates, and if it returns 0, it means that the client has disconnected.

When a new client connects to the server, the server accepts the connection using the accept function and passes the socket
descriptor to the manage_client_thread function. Before creating the new thread, the server uses the pthread_create function 
to create a new thread and pass the socket descriptor as an argument to the thread. The thread ID is stored in a vector, so 
that it can be joined later when the server is terminated.

Using pthreads in this way allows the server to manage multiple client connections simultaneously, without blocking on any
one client's input or output. This is important for the server to be able to handle a large number of clients without becoming
unresponsive.
