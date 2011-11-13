/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/10/2011
|* Last Modified on 11/12/2011
***/

#ifndef SERVER_H
#define SERVER_H

// ### GLOBAL CONSTANTS ########################################################
const unsigned short DEFAULT_PORT = 1300;
const unsigned short MAX_CLIENTS = 10;

// ### FUNCTION DECLARATIONS ###################################################
/***
|* @func  signalHandler
|* @desc	Handles abort, terminate & interupt signals.
|*
|* @param	int		signal		The signal the script recieved.
***/
void signalHandler(int signal);

/***
|* @func	handleClient
|* @desc	Handles a new client. (For use with pthread_create only!)
***/
void * handleClient(void * cid);

/***
|* @func	writeToAll
|* @desc	Writes a message to all clients.
|*
|* @param	char[]	message		The message to write to all clients.
***/
void writeToAll(const char message[]);

/***
|* @func	writeToAllOthers
|* @desc	Writes a message to all clients except the one the message is from.
|*
|* @param	char[]			message		The message to write to all clients.
|* @param	unsigned short	from		The client this message is from.
***/
void writeToAllOthers(const char message[], unsigned short from);

/***
|* @func	writeMessage
|* @desc	Formats a message before writing it to all clients except the one the message is from.
|*
|* @param	char[]			message		The message to write to all clients.
|* @param	unsigned short	from		The client this message is from.
***/
void writeMessage(const char message[], unsigned short from);

#endif
