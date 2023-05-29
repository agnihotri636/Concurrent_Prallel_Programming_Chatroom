//----------------------------------------------------------------------------------------------------
//Author : Shivam Agnihotri
//Date   : 15 April 2023
//----------------------------------------------------------------------------------------------------

#include <iostream>
#include <signal.h>         
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <net/if.h>

using namespace std;

//functions

// start the server using the configuration file
int start_server(const string & configuration_file);

// parse the user's command and take appropriate action
void parse_command(string comnd, int client_sock);

// retrieve the IP address of the server
void retrieve_ip_address();

// retrieve the port number from the configuration file
void retrieve_port_number(const string & configuration_file);

// split a string into tokens using a specified delimiter
void tokenize_string(const string& str, vector<string>& tokens, const string& delimiters);

// create a client configuration file with the server's IP address and port number
void write_config_client();

// handle the user's login command
void login(string client_name, int client_socket_no);

// handle the user's logout command
void logout(string client_name, int client_socket_no);

// send a direct message from one user to another
void send_direct_message(string from_client_name, string to_client_name, string message);

// send a broadcast message from one user to all other users
void send_broadcast_message(string from_client_name, string msg);

// print the list of currently online users
void print_online_users();

// manage a single client connection in a separate thread
void * manage_client_thread(void * socket);

// handle the termination signal to gracefully shut down the server
void handle_termination_signal(int sig_num);

// global variables

// IP address of the server
string IP;

// port number for the server
string PORT;

// list of connected clients, with usernames as keys and socket descriptors as values
map <string,int> clients;

// socket descriptor for the server
int server_socket;

// structure for client's socket address
struct sockaddr_in client_address;


// start the server
int start_server(const string & configuration_file) {
     
        //create socket
        // get the IP address of the server
        retrieve_ip_address();

        // initialize the server socket
        int socket_server;
        if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                cerr << "Error in creating socket" << endl;
                exit(EXIT_FAILURE);
        }

        //get port number from the configuration file
        retrieve_port_number(configuration_file);

        // bind the socket to the IP address and port number
        struct sockaddr_in addr;
        memset( & addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = stoi(PORT);
        addr.sin_addr.s_addr = inet_addr(IP.c_str());
        if (::bind(socket_server, (struct sockaddr * ) & addr, sizeof(addr)) < 0) {
                cerr << "Bind error" << endl;
                exit(EXIT_FAILURE);
        }

        // print server details
        cout << "" << endl;
        cout << "_________________________________________________" << endl;
        cout << "SERVER STARTED" << endl;
        cout << "Server IP:" << inet_ntoa(addr.sin_addr) << endl;
        cout << "Server Port:" << addr.sin_port << endl;
        cout << "_________________________________________________" << endl;
        cout <<endl;

        //listen for clients
        if (listen(socket_server, 10) < 0) {
               cerr << "Error in listen" << endl;
               exit(EXIT_FAILURE);
        }

        cout << "->Socket creation done" << endl;
        cout << "->Waiting for client to connect ..... " << endl;

        // create a client configuration file
         write_config_client();

        // save the server socket descriptor
        server_socket = socket_server;

        return socket_server;
}

// function to retrieve the IP address of the server
void retrieve_ip_address() {

   // retrieve the IP address of the server
struct ifaddrs * ifaddr;
if (getifaddrs(&ifaddr) == -1) {
    cerr << "Error in retrieving network interface addresses" << endl;
    exit(EXIT_FAILURE);
}

// loop through the list of network interfaces
for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

    // check if the network interface is IPv4 and not a loopback interface
    if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET &&

        !(ifa->ifa_flags & IFF_LOOPBACK)) {

        // convert the IP address to a string
        char addr_buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr,
                  addr_buf, INET_ADDRSTRLEN);
        IP = addr_buf;
        break;
    }
}

// free the memory allocated by getifaddrs()
freeifaddrs(ifaddr);
}

//read port number
void retrieve_port_number(const string & configuration_file) {

        ifstream config_info(configuration_file);
        if (!config_info.is_open()) {
                cout << "Cannot open configuration file: " << configuration_file << endl;
                exit(EXIT_FAILURE);
        }

        string line;
        while (getline(config_info, line)) {
                vector < string > config;
                tokenize_string(line, config, ":");
                if (config[0] == "port") {
                        PORT = config[1];
                        break;
                }
        }
}



//write back config file
void write_config_client() {

        ofstream outfile("config_client");
        if (!outfile.is_open()) {
                cout << "Cannot create client configuration file.\n";
                exit(EXIT_FAILURE);
        }
        string config_info = "servhost: " + IP + "\nservport: " + PORT + "\n";
        outfile << config_info << endl;
        outfile.close();
}



//split string

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


void parse_command(string command, int client_sock) {

        vector < string > arguments;
        tokenize_string(command, arguments, "<>");

        if (arguments[0] == "login") {
                login(arguments[1], client_sock);
        } else if (arguments[0] == "logout") {
                logout(arguments[1], client_sock);
        } else if (arguments[0] == "DIRECT") {
                arguments[2].erase(arguments[2].begin());
                send_direct_message(arguments[1], arguments[2], arguments[3]);
        } else if (arguments[0] == "BROADCAST") {
                send_broadcast_message(arguments[1], arguments[2]);
        }
          else {// If the command is not recognized, print an error message
                 cout << "Invalid command: " << command << endl;
    }

}

// handle the user's direct message command
void send_direct_message(string from_client_name, string to_client_name, string msg) {

        map < string, int > ::const_iterator it;

        it = clients.find(from_client_name);
        int from_client_socket = it -> second;

        it = clients.find(to_client_name);
        int to_client_socket = it -> second;

        string chat_to_info = "DIRECT MSG FROM @" + from_client_name + ">>" + msg;

        ::send(to_client_socket, chat_to_info.c_str(), (int) chat_to_info.size(), 0);

        string chat_back_info = "Direct message sent successfully";::send(from_client_socket, chat_back_info.c_str(), (int) chat_back_info.size(), 0);

}

// handle the user's broadcast message command
void send_broadcast_message(string from_client_name, string msg) {

        map < string, int > ::const_iterator it;
        it = clients.find(from_client_name);
        int from_client_socket = it -> second;

        string chat_to_info = "BROADCAST FROM  @" + from_client_name + ">>" + msg;
        
        for (auto user: clients) {
                if (user.first != from_client_name) {
                        ::send(user.second, chat_to_info.c_str(), (int) chat_to_info.size(), 0);

                }
        }

        string chat_back_info = "Broadcast message sent to all users successfully";

        ::send(from_client_socket, chat_back_info.c_str(), (int) chat_back_info.size(), 0);

}


// print the list of online users
void print_online_users() {
        cout << endl;
        cout << "----------------------------------------------------" << endl;
        cout << "Online Users:" << endl;
        for (auto user: clients) {
                cout << "---> " << user.first << endl;
        }
        cout << "----------------------------------------------------" << endl;
}

// handle the user's login command
void login(string client_name, int client_sock) {

        // check if client_name already exists in clients
        if (clients.count(client_name) > 0) {
                // client_name already in use, send error message to client and close connection
                string error_msg = "Error: Username is already in use. Please run client again and enter a different username.";::send(client_sock, error_msg.c_str(), (int) error_msg.size(), 0);
               close(client_sock);

                return;
        }
        else{
        // add new client
        clients.emplace(client_name, client_sock);

        string login_info = "You are now logged in as " + client_name;::send(client_sock, login_info.c_str(), (int) login_info.size(), 0);

        print_online_users();
        }


}

// handle the user's logout command
void logout(string client_name, int client_sock) {

        map < string, int > ::iterator it;
        it = clients.find(client_name);
        clients.erase(it);

        string logout_info = "You have logged out";::send(client_sock, logout_info.c_str(), (int) logout_info.size(), 0);
        close(client_sock);

        print_online_users();
}



// handle the termination signal 
void handle_termination_signal(int sig_num) {
        shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
        cout << endl << "Server is terminated" << endl;
        exit(EXIT_SUCCESS);
}

// thread function to manage a client's connection
void * manage_client_thread(void * socket) {
        int client_socket_no = * ((int * ) socket);
        int socket_length;
        char packet[256];

        while (1) {
                
                memset(packet, 0, sizeof(packet));
                socket_length = (int)::recv(client_socket_no, packet, 256, 0);

                if (socket_length > 0) {

                        string comnd = packet;

                        cout << comnd << endl;
                        parse_command(comnd, client_socket_no);
                } else if (socket_length == -1) {
                        cout << "Thread terminated" << endl;
                        break;
                } else {
                        cout << "socket error: " << socket_length << endl;
                        return NULL;
                }

        }

        return NULL;
}

//-------------------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------MAIN---------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------------------------//

int main(int argc, char * argv[]) {

        if (argc < 2) {
                cerr << "Usage: " << argv[0] << " <configuration_file>" << endl;
                return 1;
        }
        string configuration_file = argv[1];
        server_socket = start_server(configuration_file);

        signal(SIGINT, handle_termination_signal);
        int sock;
        vector < pthread_t > pthread_id;
        int thread_num = 0;
        int thread_create_result;

        while (1) {
                socklen_t len = sizeof(struct sockaddr_in);
                sock = ::accept(server_socket, (struct sockaddr * ) & client_address, & len);
                if (sock < 0) {
                        cerr << "Accept error" << endl;
                        continue;
                }

                pthread_t temp;
                thread_create_result = pthread_create( & temp, NULL, manage_client_thread, & sock);
                pthread_id.push_back(temp);

                if (thread_create_result != 0) {
                        cout << endl;
                        cerr << "create thread failed!" << endl;
                        cout << "                                                                           " << endl;
                        close(sock);
                } else {
                        cout << endl;
                        cout << "Thread[" << thread_num << "] created successfully" << endl;
                        cout << "                                                                           " << endl;
                }
                thread_num++;

        }

}
