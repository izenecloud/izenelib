#include <iostream>
#include <fstream>

#include <cstdio>
#include <sys/time.h>

using namespace std;

//const char* filename = "data.txt";
//const char* filename = "/mnt/mfs/data.txt";
//const char* filename = "/mnt/hdfs/data.txt";

double getNowMS()
{
    timeval now;
    gettimeofday(&now,0);

    time_t s = now.tv_sec;
    time_t mms = now.tv_usec;

    double ret = s + (double)mms/1000000.0;
    return ret;
}

void t_create(const char* file)
{
    FILE* fp = NULL;

    fp = fopen(file,"w");
    if (fp == NULL)
    {
        cout << "failed to create file"<<endl;
        return;
    }

    const char* ch = "hello world!";
    while (*ch)
    {
        fputc(*ch++, fp);
    }

    char a[7]=" ZZZZ\n";
    fwrite(a, sizeof(char), 7, fp);

    fclose(fp);
}

void t_read(const char* file)
{
    FILE* fp = NULL;

    fp = fopen(file,"r");

    if (fp == NULL)
    {
        cout << "failed to open file" <<endl;
        return;
    }

    string data;
    char c;

    while ((c = fgetc(fp)) != EOF)
    {
        data+=c;
    }

    cout << "read: "<<data<<endl;

    // fread
    char buffer[256];
    memset(buffer, 0, 256);
    char *p = buffer;

    fseek(fp, 0, SEEK_SET);
    while (fread(p, sizeof(char), 1, fp) >= 1)
    {
        cout << *p;
    }
    cout <<endl;
}

bool copyfile(const char* src, const char* dest, int buffer_size)
{
    cout << "copy file from \"" << src << "\" to \"" << dest<<"\""<<endl;

    char* buffer = new char[buffer_size];
    memset(buffer, 0, buffer_size);
    if (!buffer)
    {
        cout << "mem error" <<endl;
        return false;
    }

    double t1, t2;

    // src
    ifstream in;
    t1 = getNowMS();
    in.open(src, ios_base::in|ios_base::binary);
    if (in.bad())
    {
        cout << "failed to open " << src <<endl;
        return false;
    }
    t2 = getNowMS();
    cout << "open src file in "<<t2-t1<<"s"<<endl;

    // dest

    ifstream out_in;
    out_in.open(dest, ios_base::in);
    if (out_in.good())
    {
        out_in.close();
        double t3 = getNowMS();
        remove(dest);
        double t4 = getNowMS();
        cout << "** remove existed file: " << dest << ", cost "<<t4-t3<<"s"<<endl;
    }
    out_in.close();

    ofstream out;
    t1 = getNowMS();
    out.open(dest, ios_base::out|ios_base::binary);
    if (out.bad())
    {
        cout << "failed to create " << dest <<endl;
        return false;
    }
    t2 = getNowMS();
    cout << "open dest file in "<<t2-t1<<"s"<<endl;

    // copy
    t1 = getNowMS();
    streamsize len = 0;
    double t3 = getNowMS();
    in.read(buffer, buffer_size);
    double t4 = getNowMS();
    cout << "read in " << (t4-t3) << "s, ";
    while ((len = in.gcount()) > 0)
    {
        cout << " len "<<len<<"B, ";
        double t1 = getNowMS();
        out.write(buffer, len);
        double t2 = getNowMS();
        cout << " wrote in " << (t2-t1) << "s"<< endl;
        double t3 = getNowMS();
        in.read(buffer, buffer_size);
        double t4 = getNowMS();
        cout << "read in " << (t4-t3) << "s, ";
    }

    in.close();
    out.close();
    t2 = getNowMS();
    cout << "copy finished in " << (t2-t1) << "s"<< endl;

    delete[] buffer;
    return true;
}

void create(const char* file)
{
    t_create(file);
    t_read(file);
}


int main(int argv, char** argc)
{
    if ( argv < 3 )
    {
        cout << "Usage: ./run [create|copy] [filename|copy_src] [copy_dest] [copy_buffer_size]" <<endl;
    }

    if ( (argv >= 3) && (strcmp(argc[1],"create") == 0) )
    {
        create(argc[2]);
    }

    if ( (argv >= 4) && (strcmp(argc[1],"copy") == 0) )
    {
        int buffer_size = 1024;
        if (argv >= 5)
            buffer_size = atoi(argc[4]);

        copyfile(argc[2], argc[3], buffer_size);
    }

    return 0;
}
