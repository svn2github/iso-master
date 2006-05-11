#include <stdio.h>
#include <stdbool.h>

#include "bkError.h"

struct MessageStruct
{
    int number;
    char* text;
};

const struct MessageStruct messageStructs[] = 
{
    {
        BKERROR_READ_GENERIC,
        BKERROR_READ_GENERIC_TEXT
    },
    {
        BKERROR_END,
        BKERROR_END_TEXT
    }
};

void outputError(int errorNumIn)
{
    int count;
    bool found;
    
    found = false;
    for(count = 0; !found && messageStructs[count].number != BKERROR_END; count++)
    {
        if(messageStructs[count].number == errorNumIn)
        {
            found = true;
            
            if(errorNumIn >= BKERROR_MIN_ID && errorNumIn <= BKERROR_MAX_ID)
                fprintf(stderr, "Error: ");
            else
                fprintf(stderr, "Warning: ");
            
            fprintf(stderr, "%s\n", messageStructs[count].text);
        }
    }
    
    if(!found)
        fprintf(stderr, "Unknown error has occured\n");
}
