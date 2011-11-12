For information about the assignment, see assignment3.pdf

Instructions for Compilation:
GCC: g++ -O3 client.cpp -o client -lpthread
GCC: g++ -O3 server.cpp -o server -lpthread
Clang-LLVM: clang++ -O3 client.cpp -o client -lpthread
Clang-LLVM: clang++ -O3 server.cpp -o server -lpthread

Testing:
GCC: g++ -O3 -Wall -W -pedantic-errors client.cpp -o client -lpthread
GCC: g++ -O3 -Wall -W -pedantic-errors server.cpp -o server -lpthread
Clang-LLVM: clang++ -O3 -Wall -W -pedantic-errors client.cpp -o client -lpthread
Clang-LLVM: clang++ -O3 -Wall -W -pedantic-errors server.cpp -o server -lpthread

Instructions for Using the Program:
Launch the server executable from a host machine, then clients can use their
executable to communicate with the server.

License
Licensed under the Academic Free License version 3.0
