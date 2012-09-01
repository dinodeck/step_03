#ifndef IO_H
#define IO_H

// Helpful IO functions
class IO
{
public:
    static bool FileExists(const char* path);
private:
    IO();
    ~IO();
};


#endif