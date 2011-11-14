/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/12/2011
***/

// ### INCLUDES ################################################################
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <netdb.h>
#include "client.h"

using namespace std;

// ### GLOBAL VARIABLES ########################################################
int socketfd; 
pthread_t readThread;
char nick[31];

// #############################################################################
// ############################# START MAIN SCRIPT #############################
// #############################################################################
int main(int argc, char * argv[])
{
	// ### LOAD PROGRAM PARAMETERS #############################################
	// Check the program arguments for nick & server settings
	if (argc < 3)
	{
		cerr << "Usage is: \"" << argv[0] << " nick server [port]\"" << endl;
		return 1;
	}

	strcpy(nick, argv[1]);

	// Check the program arguments for port setting
	unsigned short port = DEFAULT_PORT;

	if (argc >= 4)
	{
		port = strtol(argv[3], NULL, 10);
	}

	// ### WATCH FOR INTERUPT SIGNAL ###########################################
	signal(SIGINT, &interuptHandler);

	// ### CONNECT TO THE SERVER ###############################################
	struct sockaddr_in server_addr = { AF_INET, htons(port) }; 
	struct hostent * host; 

	// Let the user know we are trying to connect
	cout << "* Connecting to " << argv[2] << ":" << port << " as " << nick << "... *" << endl;

	// Get the host
	if ((host = gethostbyname(argv[2])) == NULL)
	{
		cerr << "Error: Unknown Host!" << endl;
		return 1;
	}

	bcopy(host->h_addr_list[0], (char *) &server_addr.sin_addr, host->h_length);

	// Create a stream socket 
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		cerr << "Error: Socket Creation Failed!" << endl;
		return 1;
	}

	// Connect the socket
	if (connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
	{
		cerr << "Error: Connect Failed!" << endl;
		return 1;
	}

	// Send our nick to the server
	write(socketfd, nick, strlen(nick));

	// Let the user know we have connected
	cout << "* Connection Successful! *" << endl;

	// ### WATCH FOR DATA FROM THE SERVER ######################################
	if (pthread_create(&readThread, NULL, readFromServer, NULL) != 0)
	{
		// Failed to create the thread, so kill the connection
		cerr << "Error: Read thread creation failed!" << endl;
		close(socketfd);
		return 1;
	}

	// ### WATCH FOR DATA FROM THE CONSOLE #####################################
	char buffer[226];

	while (1)
	{
		cout << nick << ": ";
		cin >> buffer;

		// Check if they are wanting to see the about popup
		if (strcmp(buffer, "/about") == 0)
		{
			about();
		}

		// Check if they are wanting to quit the application
		else if (strcmp(buffer, "/exit") == 0 || strcmp(buffer, "/quit") == 0 || strcmp(buffer, "/part") == 0)
		{
			close(socketfd);
			return 0;
		}

		// Else, send the input to the server and show it locally
		else
		{
			write(socketfd, buffer, strlen(buffer));
		}
	}

	// ### DISCONNECT FROM THE SERVER ##########################################
	// Should never reach this, but just in case
	close(socketfd);

	return 0;
}
// #############################################################################
// ############################## END MAIN SCRIPT ##############################
// #############################################################################

// ### FUNCTION DEFINITIONS ####################################################
/***
|* @func	interuptHandler
|* @desc	Handles interupt signals.
|*
|* @param	int		signal		The signal the script recieved.
***/
void interuptHandler(int signal)
{
	// Let the user know the server is going down
	cout << "\r* Error: You must use \"/exit\", \"/quit\" or \"/part\" to exit! *" << endl;
	cout << nick << ": " << flush;
}

/**
|*	@func	about
|*	@desc	Displays information about the program.
**/
void about()
{
	cout << "***********************************************" << endl;
	cout << "*              About Simple Chat              *" << endl;
	cout << "*                                             *" << endl;
	cout << "* Authors: Brian Yarbrough & Madison Solarana *" << endl;
	cout << "* Version: 1.0.0                              *" << endl;
	cout << "***********************************************" << endl;

	return;
}

/***
|* @func	readFromServer
|* @desc	Watches for data from the server. (For use with pthread_create only!)
***/
void * readFromServer(void * dmyptr)
{
	char buffer[226];
	memset(buffer, '\0', 226);

	// Read from the server
	while (read(socketfd, buffer, 225))
	{
		// Check for server codes
		if (strlen(buffer) == 11 && strcmp(strstr(buffer, " *"), " *") == 0)
		{
			char temp[7];

			if (strcmp(strncpy(temp, buffer, 7), "* CODE ") == 0)
			{
				if (strcmp(buffer, "* CODE 00 *") == 0)
				{
					// Server is shutting down, let the user know
					cout << "* Server shutting down! *" << endl;
				}
				else if (strcmp(buffer, "* CODE 01 *") == 0)
				{
					// Connection was closed becuase too many clients have connected already
					cout << "* Connection closed! (Max Clients Exceeded) *" << endl;
				}
				else if (strcmp(buffer, "* CODE 02 *") == 0)
				{
					// Connection was closed becuase someone else is already using the nick requested
					cout << "* Connection closed! (Nick Already In Use) *" << endl;
				}
				else if (strcmp(buffer, "* CODE 03 *") == 0)
				{
					// Connection was closed becuase of server failure
					cout << "* Connection closed! (Internal Server Failure) *" << endl;
				}
				else
				{
					// Should never hit this, but just in case, let the user know what code was sent
					cout << "* Unknown Server Code Received! (" << buffer[8] << buffer[9] << ") *";
				}
				
				// Close the socket and exit (All known server codes will result in the server closing the connection with the client)
				close(socketfd);
				exit(0);
			}
		}

		// Correct for long usernames cuasing junk to be shown
		if (strlen(buffer) < (strlen(nick) + 1))
		{
			for (unsigned short i = strlen(buffer); i < (strlen(nick) + 1); i++)
			{
				buffer[i] = ' ';
			}
		}

		// Show the text & reset the buffer
		cout << "\r" << buffer << endl;

		// Reset the console for the next input/output
		cout << nick << ": " << flush;
		memset(buffer, '\0', 226);
	}

	// Close out this thread
	pthread_exit(NULL);
}
