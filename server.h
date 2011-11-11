#ifndef SERVER_H
#define SERVER_H

const unsigned short DEFAULT_PORT = 1323;
const unsigned short MAX_CLIENTS = 2;

void signalHandler(int sig);
void * handleClient(void * ptr);
void writeToAll(const char content[256]);

#endif
