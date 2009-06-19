#include <iostream>
#include <cstring>
#include <febird/io/BzipStream.h>

int main(int argc, char *argv[])
{
    febird::BzipOutputStream out("test");
    const char* hello = "Hello, World!";
    out.write(hello, std::strlen(hello));
    return 0;
}

