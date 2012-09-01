#include "IO.h"
#include <sys/stat.h>

bool IO::FileExists(const char* path)
{
    struct stat attributes;

    if( stat(path, &attributes) != 0 )
    {
        //printf("Error checking if file exists: [%s]", path);
        return false;
    }

    return S_ISREG(attributes.st_mode);
}