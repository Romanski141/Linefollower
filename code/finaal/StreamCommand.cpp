#include <Arduino.h>
#include "StreamCommand.h"

char *strtok_r(char *str, const char *delim, char **saveptr);

StreamCommand::StreamCommand(Stream &stream)
{
	serial = &stream;
	strncpy(delim," ",MAXDELIMETER);  // strtok_r needs a null-terminated string
	term='\r';   // return character, default terminator for commands
	numCommand=0;    // Number of callback handlers installed
	clearBuffer(); 
}

void StreamCommand::clearBuffer()
{
	for (int i=0; i < COMMANDBUFFER; i++) 
	{
		buffer[i]='\0';
	}
	bufPos=0; 
}

char *StreamCommand::next() 
{
	char *nextToken;
	nextToken = strtok_r(NULL, delim, &last); 
	return nextToken; 
}

void StreamCommand::readStream() 
{
	while (serial->available() > 0) 
	{
		int i; 
		bool matched; 
		inChar=serial->read();   // Read single available character, there may be more waiting
	
		if (inChar==term)      // Check for the terminator (default '\r') meaning end of command
		{
			bufPos=0;           // Reset to start of buffer
			token = strtok_r(buffer,delim,&last);   // Search for command at start of buffer
			if (token == NULL) return; 
			matched=false; 
			for (i=0; i<numCommand; i++)
                        {
				// Compare the found command against the list of known commands for a match
				if (strncmp(token,CommandList[i].command,COMMANDBUFFER) == 0) 
				{
					// Execute the stored handler function for the command
					(*CommandList[i].function)(); 
					clearBuffer(); 
					matched=true; 
					break; 
				}
			}
			if (matched==false)
                        {
				if (defaultHandler != NULL) (*defaultHandler)(token); 
				clearBuffer(); 
			}

		}
		if (isprint(inChar))   // Only printable characters into the buffer
		{
			buffer[bufPos++]=inChar;   // Put character into buffer
			buffer[bufPos]='\0';  // Null terminate
			if (bufPos > COMMANDBUFFER-1) bufPos=0; // wrap buffer around if full  
		}
	}
}

void StreamCommand::addCommand(const char *command, void (*function)())
{
	if (numCommand < MAXCOMMANDS) 
    {
		strncpy(CommandList[numCommand].command,command,COMMANDBUFFER); 
		CommandList[numCommand].function = function; 
		numCommand++; 
	} 
}

void StreamCommand::setDefaultHandler(void (*function)(char*))
{
	defaultHandler = function;
}
