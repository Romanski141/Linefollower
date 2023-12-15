#ifndef StreamCommand_h
#define StreamCommand_h

#define COMMANDBUFFER 32
#define MAXCOMMANDS 10
#define MAXDELIMETER 2

#include <Stream.h>

class StreamCommand
{
	public:
		StreamCommand(Stream &stream);  // Constructor for using SoftwareSerial objects

		void clearBuffer();   // Sets the command buffer to all '\0' (nulls)
		char *next();         // returns pointer to next token found in command buffer (for getting arguments to commands)
		void readStream();    // Main entry point.  
		void addCommand(const char *, void(*)());   // Add commands to processing dictionary
		void setDefaultHandler(void (*function)(char*));    // A handler to call when no valid command received. 
	
	private:
		char inChar;          // A character read from the serial stream 
		char buffer[COMMANDBUFFER];   // Buffer of stored characters while waiting for terminator character
		int  bufPos;                        // Current position in the buffer
		char delim[MAXDELIMETER];           // null-terminated list of character to be used as delimeters for tokenizing (default " ")
		char term;                          // Character that signals end of command (default '\r')
		char *token;                        // Returned token from the command buffer as returned by strtok_r
		char *last;                         // State variable used by strtok_r during processing
		typedef struct _callback {
			char command[COMMANDBUFFER];
			void (*function)();
		} StreamCommandCallback;            // Data structure to hold Command/Handler function key-value pairs
		int numCommand;
		StreamCommandCallback CommandList[MAXCOMMANDS];   // Actual definition for command/handler array
		void (*defaultHandler)(char*);           // Pointer to the default handler function 
		Stream *serial;         // Pointer to a user-created SoftwareSerial object
};

#endif