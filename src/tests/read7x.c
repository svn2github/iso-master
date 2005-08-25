#include <unistd.h>

int read711(int file, unsigned char* value)
{
    return read(file, value, 1);
}

int read712(int file, signed char* value)
{
    return read(file, value, 1);
}

int read721(int file, unsigned short* value)
{
    return read(file, value, 2);
}

int read722(int file, unsigned short* value)
{
    int rc;
    char byte;
    
    rc = read(file, value, 2);
    if(rc != 2)
        return rc;
    
    byte = *value >> 8;
    *value <<= 8;
    *value |= byte;
    
    return rc;
}

int read723(int file, unsigned short* value)
{
    int rc;
    short trash;
    
    rc = read(file, value, 2);
    if(rc != 2)
        return rc;
    
    return read(file, &trash, 2);
}

int read731(int file, unsigned* value)
{
    return read(file, value, 4);
}

int read732(int file, unsigned* value)
{
    int rc;
    char byte2;
    char byte3;
    char byte4;
    
    rc = read(file, value, 4);
    if(rc != 4)
        return rc;
    
    byte2 = *value >> 8;
    byte3 = *value >> 16;
    byte4 = *value >> 24;
    
    *value <<= 8;
    *value |= byte2;
    *value <<= 8;
    *value |= byte3;
    *value <<= 8;
    *value |= byte4;
    
    return rc;
}

int read733(int file, unsigned* value)
{
    int rc;
    int trash;
    
    rc = read(file, value, 4);
    if(rc != 4)
        return rc;
    
    return read(file, &trash, 4);
}
