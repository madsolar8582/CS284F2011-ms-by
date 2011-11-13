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
bool forever = true;
int socketfd; 

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

	// Check the program arguments for port setting
	unsigned short port = DEFAULT_PORT;

	if (argc >= 4)
	{
		port = strtol(argv[1], NULL, 10);
	}

	// ### WATCH FOR INTERUPT SIGNAL ###########################################
	signal(SIGINT, &interuptHandler);

	// ### CONNECT TO THE SERVER ###############################################
	struct sockaddr_in server_addr = { AF_INET, htons(port) }; 
	struct hostent * host; 

	// Let the user know we are trying to connect
	cout << "* Connecting to " << argv[2] << ":" << port << " as " << argv[1] << "... *" << endl;

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
    write(socketfd, argv[1], strlen(argv[1]));

	// Let the user know we have connected
	cout << "* Connection Successful! *" << endl;

	// ### WATCH FOR DATA FROM THE SERVER ######################################
	// create a new thread to watch for data

	// ### WATCH FOR DATA FROM THE CONSOLE #####################################
    char buffer[226];

	while (forever)
	{
		cin >> buffer;

		// Check if they are wanting to see the about popup
		if (strcmp(buffer, "/about") == 0)
		{
			about();
		}

		// Check if they are wanting to quit the application
		else if (strcmp(buffer, "/exit") == 0 || strcmp(buffer, "/quit") == 0 || strcmp(buffer, "/part") == 0)
		{
			forever = false;
		}

		// Else, send the input to the server and show it locally
		else
		{
			write(socketfd, buffer, strlen(buffer));
			cout << argv[1] << ": " << buffer;
		}
	}

	// ### DISCONNECT FROM THE SERVER ##########################################
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
	cout << "* Error: You must use \"/exit\", \"/quit\" or \"/part\" to exit! *" << endl;
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
	cout << "* Version: 0.1.0                              *" << endl;
	cout << "***********************************************" << endl;

	return;
}
