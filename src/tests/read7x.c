#include <unistd.h>

int read711(int image, unsigned char* value)
{
    return read(image, value, 1);
}

int read712(int image, signed char* value)
{
    return read(image, value, 1);
}

int read721(int image, unsigned short* value)
{
    return read(image, value, 2);
}

int read722(int image, unsigned short* value)
{
    int rc;
    char byte;
    
    rc = read(image, value, 2);
    if(rc != 2)
        return rc;
    
    byte = *value >> 8;
    *value <<= 8;
    *value |= byte;
    
    return rc;
}

int read723(int image, unsigned short* value)
{
    int rc;
    short trash;
    
    rc = read(image, value, 2);
    if(rc != 2)
        return rc;
    
    rc = read(image, &trash, 2);
    if(rc != 2)
        return rc;
    
    return 4;
}

int read731(int image, unsigned* value)
{
    return read(image, value, 4);
}

int read732(int image, unsigned* value)
{
    int rc;
    char byte2;
    char byte3;
    char byte4;
    
    rc = read(image, value, 4);
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

int read733(int image, unsigned* value)
{
    int rc;
    int trash;
    
    rc = read(image, value, 4);
    if(rc != 4)
        return rc;
    
    rc = read(image, &trash, 4);
    if(rc != 4)
        return rc;
    
    return 8;
}
