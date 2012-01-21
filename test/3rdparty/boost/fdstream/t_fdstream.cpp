#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <3rdparty/boost/fdstream.hpp>

void test_fdostream()
{
    boost::fdostream out(1);    // stream with buffer writing to file descriptor 1

    out << "31 hexadecimal: " << std::hex << 31 << std::endl;
}

void test_fdstreams()
{
    boost::fdistream in(0);     // stream with buffer reading from file descriptor 0
    boost::fdostream out(1);    // stream with buffer writing to file descriptor 1

    out << "read 20 chars and print them (2 twice due to use of unget())"
        << std::endl;

    char c;
    for (int i=1; i<=20; i++) {
        // read next character (out of the buffer)
        in.get(c);

        // print that character (and flush)
        out << c << std::flush;

        // after eight characters, put two characters back into the stream
        if (i == 8) {
            in.unget();
            in.unget();
        }
    }
    out << std::endl;
}

void copyfile (const std::string& infile, const std::string& outfile)
{
    int fdin, fdout;

#ifdef _MSC_VER
    fdin = open( infile.c_str(), O_RDONLY | _O_BINARY);
#else
    fdin = open( infile.c_str(), O_RDONLY);
#endif
    if (fdin < 0) {
        throw "open failed on input file";
    }
    std::cout << "open succeeded on input file\n";

#ifdef _MSC_VER
    fdout = open( outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
                                   S_IREAD | S_IWRITE );
#else
    fdout = open( outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                                   S_IREAD | S_IWRITE );
#endif
    if (fdout == -1) {
        throw "open failed on output file";
    }
    std::cout << "open succeeded on output file\n";
    
    // copy all standard input to standard output
    boost::fdistream in(fdin);
    boost::fdostream out(fdout);
    out << in.rdbuf();

    close (fdin);
    close (fdout);
}


void popen_test (std::string const& command)
{
    FILE* fp;

    // open pipe to read from
    if ((fp=popen(command.c_str(),"r")) == NULL) {
        throw "popen() failed";
    }

    // and initialize input stream to read from it
    boost::fdistream in(fileno(fp));

    // print all characters with indent
    std::cout << "output of " << command << ":\n";
    char c;
    while (in.get(c)) {
        std::cout.put(c);
        if (c == '\n') {
            std::cout.put('>');
            std::cout.put(' ');
        }
    }
    std::cout.put('\n');

    pclose(fp);
}

int main()
{
    popen_test("ls -l");
    popen_test("dir");
    popen_test("date");
    test_fdostream();
    test_fdstreams();
    try {
        std::string infile="fdstream.hpp";
        std::string outfile="tmp.out";

        std::cout << "low-level I/O from " << infile
                  << " to " << outfile << std::endl;
        copyfile(infile,outfile);
    }
    catch (const char* s) {
        std::cerr << "EXCEPTION: " << s << std::endl;
    }
}

