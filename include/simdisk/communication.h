#ifndef __COMMAND_H
#define __COMMAND_H

// used for communication between simdisk and simshell
// simshell -> simdisk: send command
// simdisk -> simshell: return result
class ShareMemory {
public:
	int size;
	char buffer[4096];
};

// used for send request to create a connection
class Requset {
public:
	constexpr static int req_type = 1;
	constexpr static int req_size = 104; // sizeof(Request) - sizeof(type)

	long type; // message type
	int pid;   // pid of the process that send the message
	// int uid;   // user id

	char user[32];	   // user name
	char password[64]; // password
};

#endif // __COMMAND_H