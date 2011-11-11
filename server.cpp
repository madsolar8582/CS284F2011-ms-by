
/*
This is what I have gotten done so far...
Note: something in handleClient() is not working correctly, so the server isn't seeing the clients responses
*/

#include <iostream>
#include <cstdlib>
#include <csignal>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "server.h"

using namespace std;

// Global Variables
int socketfd, clientSockets[MAX_CLIENTS];
pthread_mutex_t clientMutex;
unsigned short currentConnections = 0;
pthread_t threads[MAX_CLIENTS];

int main(int argc, char * argv[])
{
  // Check the program arguments for port setting
	unsigned short port = DEFAULT_PORT;

	if (argc >= 2)
	{
		port = strtol(argv[1], NULL, 10);
	}

	// Create a stream socket
	int newsfd;
	struct sockaddr_in server_addr = { AF_INET, htons(port) };
	struct sockaddr_in client_addr = { AF_INET };
	socklen_t client_len = sizeof(client_addr);

	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cerr << "* Error: Socket Creation Failed! *" << endl;
		return 1;
	}

	// Bind the socket to an internet port
	if (bind(socketfd, (struct sockaddr *) & server_addr, sizeof(server_addr)) == -1)
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

	// Watch for abort, terminate or interupt signals
    signal(SIGABRT, & signalHandler);
	signal(SIGTERM, & signalHandler);
	signal(SIGINT, & signalHandler);

	// Loop forever
	while ((newsfd = accept(socketfd, (struct sockaddr *) & client_addr, & client_len)) > 0)
	{  
		// Check if we are at MAX_CLIENTS currently
		if (currentConnections >= MAX_CLIENTS)
		{
			cout << "* Connection to host refused! (Max Clients Exceeded) *" << endl;
			write(newsfd, "* Connection closed! (Max Clients Exceeded) *", 45);
			close(newsfd);
		}
		else
		{
			// Update the current connection count
			pthread_mutex_lock(& clientMutex);
			clientSockets[currentConnections++] = newsfd;
			pthread_mutex_unlock(& clientMutex); 

			if (currentConnections >= MAX_CLIENTS)
			{
				cout << "* Max Clients Reached! *" << endl;
			}

			// Handle this new client
			pthread_create(& threads[currentConnections], NULL, handleClient, NULL);

		}

	}

	// Should never reach this
	close(socketfd);

	return 0;
}

void signalHandler(int sig)
{
	// Let each client know the server is going down
	writeToAll("* Server shutting down in 10 seconds! *");

	// Disallow new connections
	pthread_mutex_lock(& clientMutex);

	// Let the user know the server is going down
	cout << "* Server is shutting down... *" << endl;

	// Wait 10 seconds

	// Kill the server
	close(socketfd);
	pthread_mutex_unlock(& clientMutex);

	// Let the user know the server has shut down
	cout << "* Server has shut down! *" << endl;

	exit(0);
}

void * handleClient(void * ptr)
{
	int clientID = currentConnections;
	int socketfd = clientSockets[clientID];
	char buffer[256];

	cout << "test" << endl;
	while (read(socketfd, buffer, 255) > 0)
	
	{
		cout << "message gotten" << endl;
		writeToAll(buffer);
	}

	cout << "killing thread" << endl;
	
	// Remove this client
	pthread_mutex_lock(& clientMutex);
	clientSockets[clientID] = -1;
	pthread_mutex_unlock(& clientMutex);
	close(socketfd);
	pthread_exit(ptr);
}

void writeToAll(const char content[256])
{
	pthread_mutex_lock(& clientMutex);
	for (unsigned short i = 0; i < currentConnections; i++)
	{
		if (clientSockets[i] != -1)
		{
			write(clientSockets[i], content, 255);
		}
	}
	pthread_mutex_unlock(& clientMutex);
}
