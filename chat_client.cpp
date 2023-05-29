//----------------------------------------------------------------------------------------------------
//Author : Shivam Agnihotri
//Date   : 15 April 2023
//----------------------------------------------------------------------------------------------------


#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/select.h>
#include <string>
#include <vector>
#include <sys/select.h>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <netinet/in.h>


using namespace std;

int start_client(string configuration_file);
void parse_command(string comnd);
void login();
int set_sock();
void tokenize_string(const string& str, vector<string>& tokens, const string& delimiters);
void logout();
void read_input();
void chat();
void handle_termination_signal(int sig_num);

int client_sock = -1;
string SERV_PORT; // server port number
string SERV_IP; // server IP address
string comnds; // user input command
string client_name; // user name for chat room
vector<string> argmnts; // arguments for user input command
fd_set fds_; // file descriptor set for select function
int max_fd = getdtablesize(); // maximum number of file descriptors

// Function to start a socket connection with the server
int start_client(string configuration_file) {


        int socket_client;
        struct sockaddr_in addr;
        if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                cerr << "Can't get socket" << endl;
                exit(EXIT_FAILURE);
        }
         // set up the server address
        memset( & addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = stoi(SERV_PORT);
        addr.sin_addr.s_addr = inet_addr(SERV_IP.c_str());
          // connect to the server
        if (connect(socket_client, (struct sockaddr * ) & addr, sizeof(addr)) < 0) {
                cerr << "Error in connect" << endl;
                exit(EXIT_FAILURE);
        }
        // save the socket file descriptor
        client_sock = socket_client;

        return socket_client;

}


// Function to send a login message to the server with the user's username and receive a response
void login() {

        cout << "Logging in ..... Checking if username is available ....... " << endl;

        if (client_sock != -1) {
                cout << "You are already logged in as @" << client_name << endl;
                return;
        }

        start_client("config_client.conf");

        client_name = argmnts[1];

        string login_info = "login<>" + client_name;

        ::send(client_sock, login_info.c_str(), (int) login_info.size(), 0);
        
        // receive a response from the server
        char packet[256];
        memset(packet, 0, sizeof(packet));
        int socket_length = (int)::recv(client_sock, packet, 256, 0);

        // print the response to the console
        if (socket_length > 0) {
                cout << packet << endl;
        }

}

// Function to send a logout message to the server with the user's username and receive a response
void logout() {

        cout << "Logging out ....." << endl;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        string logout_info = "logout<>" + client_name;

        // send a logout message to the server with the user's username
        ::send(client_sock, logout_info.c_str(), (int) logout_info.size(), 0);

        // receive a response from the server
        char packet[256];
        memset(packet, 0, sizeof(packet));
        int socket_length = (int)::recv(client_sock, packet, 256, 0);

        // print the response to the console
        if (socket_length > 0) {
                cout << packet << endl;
        }

       // reset the socket file descriptor
        client_sock = -1;

}

// Function to send a chat message to the server to either broadcast to all users or to a specific user
void chat() {
        string chat_info;

        if (argmnts[1][0] != '@') {
                string message = "";
                for (unsigned int i = 1; i < argmnts.size(); i++) {
                        message += argmnts[i] + " ";
                }
                message.pop_back();
                chat_info = "BROADCAST<>" + client_name + "<>" + message;
        } else {

                string message = "";
                for (unsigned int i = 2; i < argmnts.size(); i++) {
                        message += argmnts[i] + " ";
                }
                message.pop_back();

                chat_info = "DIRECT<>" + client_name + "<>" + argmnts[1] + "<>" + message;
        }

        ::send(client_sock, chat_info.c_str(), (int) chat_info.size(), 0);

}

// Function to split a string into a vector of substrings based on a given delimiter
void tokenize_string(const string& str, vector<string>& tokens, const string& delimiters) {

    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first non-delimiter.
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next non-delimiter.
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// Function to parse the user's input and call the appropriate function
void parse_command(string command) {

        command.pop_back();

        comnds = command;
        argmnts.clear();
        tokenize_string(comnds, argmnts, " ");

         // check the first argument to determine which function to call
        if (argmnts[0] == "exit") {
                logout();
                shutdown(client_sock, SHUT_RDWR);
                close(client_sock);
                cout << endl << "Client exited successfully" << endl;
                exit(EXIT_SUCCESS);
        } else if (argmnts[0] == "login")//if command is login call login()
                login();
        else if (argmnts[0] == "logout")//if command is logout call logout()
                logout();
        else if (argmnts[0] == "chat") //if command is chat call chat()
                chat();
        else {
                 cout << "Invalid command: " << command << endl;// If the command is not recognized, print an error message
    }

}


// Function to read the user's input from the command line
void read_input() {

        comnds.clear();
        int temp = 0;

        while (temp != '\n') {
                // Read in the char
                temp = getchar();
                comnds.push_back(temp);
        }
        comnds.pop_back();
}

// Function to set the socket file descriptor for the client
int set_sock() {

        return client_sock;
}

// Function to handle the termination signal 
void handle_termination_signal(int sig_num) {
        logout();
        shutdown(set_sock(), SHUT_RDWR);
        close(set_sock());
        cout << endl << "Termination signal captured, exiting." << endl;
        exit(EXIT_SUCCESS);
}

//-------------------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------MAIN---------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------------------------//

int main(int argc, char * argv[]) {

        // set up a signal handler to capture the termination signal
        signal(SIGINT, handle_termination_signal);

        if (argc < 2) {
                cerr << "Missing config file path" << endl;
                exit(EXIT_FAILURE);
        }

        // read in the server IP address and port number from the configuration file
        ifstream config_info(argv[1]);
        if (!config_info.is_open()) {
                cerr << "Cannot open input file." << endl;
                exit(EXIT_FAILURE);
        }
        string line;
        while (getline(config_info, line)) {
                if (line != "") {
                        vector < string > config_result;
                        tokenize_string(line, config_result, ": ");
                        if (config_result[0] == "servhost")
                                SERV_IP = config_result[1];
                        else if (config_result[0] == "servport")
                                SERV_PORT = config_result[1];
                }
        }

        // print out the available commands to the user
        cout << "_______________________________________________________" << endl;
        cout << "               WELCOME TO CHAT-ROOM                    " << endl;
        cout << "_______________________________________________________" << endl;
        cout << "                     ::Commands::                      " << endl;
        cout << "Login                     : login <username>" << endl;
        cout << "Logout                    : logout <username>" << endl;
        cout << "Chat with particular user : chat <@username> <message>" << endl;
        cout << "Broadcast                 : chat <message>" << endl;
        cout << "_______________________________________________________" << endl;
        cout << "Please login and continue to chat......"<<endl;

        while (1) {

                // set up the file descriptor set for the select function
                FD_ZERO( &fds_);
                FD_SET(0, &fds_);
                if (set_sock() != -1)
                        FD_SET(set_sock(), &fds_);

                // wait for input from either the user or the server
                switch (select(max_fd, &fds_, NULL, NULL, NULL)) {
                case -1:
                        cerr << "Error in select." << endl;
                        exit(EXIT_FAILURE);
                        break;

                case 0:
                        cout << "Error" << endl;
                        break;

                default:
                        if (set_sock() != -1 && FD_ISSET(set_sock(), &fds_)) {

                                // there is input available on the socket, read it in and print to the console
                                char packet[256];
                                memset(packet, 0, sizeof(packet));
                                int socket_length = (int)::recv(set_sock(), packet, 256, 0);
                                if (socket_length > 0) {
                                        cout << packet << endl;
                                } else if (socket_length < 0) {
                                        cerr << "Error receiving." << endl;

                                        break;
                                } else {
                                        cerr << "Server exited/client terminated." << endl;
                                        exit(EXIT_FAILURE);
                                }
                        } else if (FD_ISSET(0, &fds_)) {
                                
                                // there is input available from the user, read it in and parse the command
                                char packet[256];
                                fgets(packet, 256, stdin);
                                string comnd = packet;
                                parse_command(comnd);
                        }

                }
        }

        return 0;
}