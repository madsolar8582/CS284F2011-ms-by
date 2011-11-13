/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/12/2011
***/

#ifndef CLIENT_H
#define CLIENT_H

// ### GLOBAL CONSTANTS ########################################################
const unsigned short DEFAULT_PORT = 1300;

// ### FUNCTION DECLARATIONS ###################################################
/***
|* @func  interuptHandler
|* @desc	Handles interupt signals.
|*
|* @param	int		signal		The signal the script recieved.
***/
void interuptHandler(int signal);

/**
|*	@func	about
|*	@desc	Displays information about the program.
**/
void about();

/***
|* @func	readFromServer
|* @desc	Watches for data from the server. (For use with pthread_create only!)
***/
void * readFromServer(void * dmyptr);

#endif
