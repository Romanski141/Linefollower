#include <Arduino.h>
#include "TelnetCommand.h"

TelnetCommand::TelnetCommand(Stream &stream)
{
    serial = &stream;
    strncpy(delim, " ", MAXDELIMETER);
    term = '\r';
    numCommand = 0;
    clearBuffer();
}

void TelnetCommand::clearBuffer()
{
    for (int i = 0; i < COMMANDBUFFER; i++)
    {
        buffer[i] = '\0';
    }
    bufPos = 0;
}

char *TelnetCommand::next()
{
    char *nextToken;
    nextToken = strtok_r(NULL, delim, &last);
    return nextToken;
}

void TelnetCommand::readStream(Stream &stream)
{
    while (stream.available() > 0)
    {
        int i;
        bool matched;
        inChar = stream.read();

        if (inChar == term)
        {
            bufPos = 0;
            token = strtok_r(buffer, delim, &last);
            if (token == NULL)
                return;
            matched = false;
            for (i = 0; i < numCommand; i++)
            {
                if (strncmp(token, CommandList[i].command, COMMANDBUFFER) == 0)
                {
                    (*CommandList[i].function)();
                    clearBuffer();
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                if (defaultHandler != NULL)
                    (*defaultHandler)(token);
                clearBuffer();
            }
        }
        if (isprint(inChar))
        {
            buffer[bufPos++] = inChar;
            buffer[bufPos] = '\0';
            if (bufPos > COMMANDBUFFER - 1)
                bufPos = 0;
        }
    }
}

void TelnetCommand::addCommand(const char *command, void (*function)())
{
    if (numCommand < MAXCOMMANDS)
    {
        strncpy(CommandList[numCommand].command, command, COMMANDBUFFER);
        CommandList[numCommand].function = function;
        numCommand++;
    }
}

void TelnetCommand::setDefaultHandler(void (*function)(char *))
{
    defaultHandler = function;
}