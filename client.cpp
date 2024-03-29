/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/12/2011
|* Last Modified on 11/15/2011
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
char nick[31], nickRemover[31];

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

	// Set up the nickRemover string for later use
	memset(nickRemover, '\0', 31);
	for (unsigned short i = 0; i < (strlen(nick) + 2); i++)
	{
		nickRemover[i] = '\b';
	}

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
	bool closeConnection = false;

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
					strcpy(buffer, "* Server shutting down! *");
				}
				else if (strcmp(buffer, "* CODE 01 *") == 0)
				{
					// Connection was closed becuase too many clients have connected already
					strcpy(buffer, "* Connection closed! (Max Clients Exceeded) *");
				}
				else if (strcmp(buffer, "* CODE 02 *") == 0)
				{
					// Connection was closed becuase someone else is already using the nick requested
					strcpy(buffer, "* Connection closed! (Nick Already In Use) *");
				}
				else if (strcmp(buffer, "* CODE 03 *") == 0)
				{
					// Connection was closed becuase of server failure
					strcpy(buffer, "* Connection closed! (Internal Server Failure) *");
				}
				else
				{
					// Should never hit this, but just in case, let the user know what code was sent
					char code[2] = {buffer[8], buffer[9]};
					strcpy(buffer, "* Unknown Server Code Received! (");
					strcat(buffer, code);
					strcat(buffer, ") *");
				}

				// Tell the program that we need to close now (All known server codes will result in the server closing the connection with the client)
				closeConnection = true;				
			}
		}

		// Show the text
		cout << nickRemover << buffer << endl;

		// Reset the console for the next input
		cout << nick << ": " << flush;

		// Check if we are to close the application
		if (closeConnection)
		{
			// Close the socket
			close(socketfd);

			// Wait 9 seconds
			sleep(9);

			// Remove the last username prompt from the screen
			cout << nickRemover;

			// Exit the program
			exit(0);
		}

		// Reset the buffer for the next output
		memset(buffer, '\0', 226);
	}

	// Close out this thread
	pthread_exit(NULL);
}
