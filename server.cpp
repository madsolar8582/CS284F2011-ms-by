/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/10/2011
|* Last Modified on 11/12/2011
***/

// ### INCLUDES ################################################################
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <netdb.h>
#include "server.h"

using namespace std;

// ### GLOBAL VARIABLES ########################################################
unsigned short currentConnections = 0;
int socketfd, clientSockets[MAX_CLIENTS];
pthread_mutex_t clientMutex;
pthread_t threads[MAX_CLIENTS];
char clientNicks[MAX_CLIENTS][31]; // Should not be changed!

// #############################################################################
// ############################# START MAIN SCRIPT #############################
// #############################################################################
int main(int argc, char * argv[])
{
	// ### LOAD PROGRAM PARAMETERS #############################################
	unsigned short port = DEFAULT_PORT;

	// Check the program arguments for port setting
	if (argc >= 2)
	{
		port = strtol(argv[1], NULL, 10);
	}

	// ### SETUP THE SERVER ####################################################
	struct sockaddr_in server_addr = { AF_INET, htons(port) };
	struct sockaddr_in client_addr = { AF_INET };
	socklen_t client_len = sizeof(client_addr);

	// Create a stream socket
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cerr << "* Error: Socket Creation Failed! *" << endl;
		return 1;
	}

	// Bind the socket to an internet port
	if (bind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
	{
		cerr << "* Error: Bind Failed! *" << endl;
		return 1;
	}

	// Listen for clients
	if (listen(socketfd, 1) == -1)
	{
		cerr << "* Error: Listen Failed! *" << endl;
		return 1;
	}

	// Let the user know the server is now listening for clients
	cout << "* Server is listening for clients... *" << endl;

	// ### WATCH FOR ABORT, TERMINATE & INTERUPT SIGNALS #######################
	signal(SIGABRT, &signalHandler);
	signal(SIGTERM, &signalHandler);
	signal(SIGINT, &signalHandler);

	// ### LOOK FOR CONNECTING USERS ###########################################
	int newsfd;

	while ((newsfd = accept(socketfd, (struct sockaddr *) &client_addr, &client_len)) > 0)
	{  
		// Check if we are at MAX_CLIENTS currently
		if (currentConnections >= MAX_CLIENTS)
		{
			cout << "* Connection to client refused! (Max Clients Exceeded) *" << endl;
			write(newsfd, "* CODE 01 *", 11);
			close(newsfd);
		}
		else
		{
			// Get the username of this client
			char nick[31];
			memset(nick, '\0', 31);

			if (read(newsfd, nick, 30) <= 0)
			{
				// Kill this connection since we can't read in the nick
				cout << "* Connection to client failed! *" << endl;
				close(newsfd);
				continue;
			}

			pthread_mutex_lock(&clientMutex);

			// Check that no one else is using that nick
			bool used = false;

			for (unsigned short i = 0; i < currentConnections; i++)
			{
				if (strcmp(nick, clientNicks[i]) == 0)
				{
					// Kill this connection as this nick is already in use
					cout << "* Connection to client refused! (Nick Already In Use) *" << endl;
					write(newsfd, "* CODE 02 *", 11);
					close(newsfd);
					used = true;
					break;
				}
			}

			if (used)
			{ 
				pthread_mutex_unlock(&clientMutex); 
				continue;
			}

			// Update the current connection count and add this user to the client sockets/nick arrays
			clientSockets[currentConnections] = newsfd;
			strcpy(clientNicks[currentConnections], nick);
			currentConnections++;

			pthread_mutex_unlock(&clientMutex); 

			// Handle this new client
			if (pthread_create(&threads[currentConnections], NULL, handleClient, NULL) != 0)
			{
				// Failed to create the thread, so remove the client, kill the connection and let the server know
				currentConnections--;

				cerr << "* Connection to client failed! (Failed To Create New Thread) *" << endl;
				write(newsfd, "* CODE 03 *", 11);
				close(newsfd);
			}
		}
	}

	// ### CLOSE OUT THE SERVER ################################################
	// Should never reach this, but just in case
	close(socketfd);

	return 0;
}
// #############################################################################
// ############################## END MAIN SCRIPT ##############################
// #############################################################################

// ### FUNCTION DEFINITIONS ####################################################
/***
|* @func	signalHandler
|* @desc	Handles abort, terminate & interupt signals.
|*
|* @param	int		signal		The signal the script recieved.
***/
void signalHandler(int signal)
{
	// Let each client know the server is going down
	writeToAll("* CODE 00 *");

	// Disallow new connections
	pthread_mutex_lock(&clientMutex);

	// Let the user know the server is going down
	cout << "* Server is shutting down... *" << endl;

	// Wait 10 seconds
	sleep(10);

	// Kill the server
	close(socketfd);
	pthread_mutex_unlock(&clientMutex);

	// Let the user know the server has shut down
	cout << "* Server has shut down! *" << endl;

	exit(0);
}

/***
|* @func	handleClient
|* @desc	Handles a new client. (For use with pthread_create only!)
***/
void * handleClient(void * ptr)
{
	unsigned short clientID = currentConnections - 1;
	char buffer[226];

	// Let everyone know that this user has joined
	strcpy(buffer, "* ");
	strcat(buffer, clientNicks[clientID]);
	strcat(buffer, " has connected! *");
	writeToAllOthers(buffer, clientID);
	cout << buffer << endl;

	// Check if we have hit MAX_CLIENTS
	if (currentConnections >= MAX_CLIENTS)
	{
		cout << "* Max Clients Reached! *" << endl;
	}

	// Start waiting for more responses from the client
	memset(buffer, '\0', 226);

	while (read(clientSockets[clientID], buffer, 225) > 0)
	{
		writeMessage(buffer, clientID);
		memset(buffer, '\0', 226);
	}

	// Remove this client
	pthread_mutex_lock(&clientMutex);

	close(clientSockets[clientID]);
	clientSockets[clientID] = -1;
	
	// Let everyone know that this user has disconnected
	strcpy(buffer, "* ");
	strcat(buffer, clientNicks[clientID]);
	strcat(buffer, " has disconnected! *");
	strcpy(clientNicks[clientID], "");

	pthread_mutex_unlock(&clientMutex);

	writeToAllOthers(buffer, clientID);
	cout << buffer << endl;

	// Close out this thread
	pthread_exit(ptr);
}

/***
|* @func	writeToAll
|* @desc	Writes a message to all clients.
|*
|* @param	char[]	message		The message to write to all clients.
***/
void writeToAll(const char message[])
{
	pthread_mutex_lock(&clientMutex);

	for (unsigned short i = 0; i < currentConnections; i++)
	{
		if (clientSockets[i] != -1)
		{
			write(clientSockets[i], message, strlen(message));
		}
	}

	pthread_mutex_unlock(&clientMutex);
}

/***
|* @func	writeToAllOthers
|* @desc	Writes a message to all clients except the one the message is from.
|*
|* @param	char[]			message		The message to write to all clients.
|* @param	unsigned short	from		The client this message is from.
***/
void writeToAllOthers(const char message[], unsigned short from)
{
	pthread_mutex_lock(&clientMutex);

	for (unsigned short i = 0; i < currentConnections; i++)
	{
		if (clientSockets[i] != -1 && i != from)
		{
			write(clientSockets[i], message, strlen(message));
		}
	}

	pthread_mutex_unlock(&clientMutex);
}

/***
|* @func	writeMessage
|* @desc	Formats a message before writing it to all clients except the one the message is from.
|*
|* @param	char[]			message		The message to write to all clients.
|* @param	unsigned short	from		The client this message is from.
***/
void writeMessage(const char message[], unsigned short from)
{
	char send[256];

	// Append the user's nick to the message
	strcpy(send, clientNicks[from]);
	strcat(send, ": ");
	strcat(send, message);

	// Write the message to everyone else
	writeToAllOthers(send, from);
}
