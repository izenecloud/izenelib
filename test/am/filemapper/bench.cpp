#include <ctime>
#include <iostream>
#include <cassert>

#include <am/filemapper/persist_stl.h>


namespace izenelib {namespace am{
struct AddressBook
{
    struct Person
    {
        // string address;
        fixed_string<30> address;
        fixed_string<14> telephone;
    };

    typedef mapped_map<fixed_string<20>, Person> Pmap;
    Pmap addresses;
};
}}

namespace std
{

struct AddressBook
{
    struct Person
    {
        // string address;
        izenelib::am::fixed_string<30> address;
        izenelib::am::fixed_string<14> telephone;
    };

    typedef map<izenelib::am::fixed_string<20>, Person> Pmap;

    Pmap addresses;
};
}

using namespace std;
using namespace izenelib::am;

template<class AddressBook>
void create_persist(AddressBook &addresses, int num)
{
    addresses.addresses.clear();

    for (int i=0; i<num; ++i)
    {
        typename AddressBook::Person p;
        char name[30], address[60], phone[15];
        sprintf(name, "Person #%d", i);
        sprintf(address, "%d nowhere avenue", i);
        sprintf(phone, "07886 %d", i);

        // p.name = name;
        p.address = address;
        p.telephone = phone;
        addresses.addresses[name] = p;
    }
}

template<class AddressBook>
void read_seq_persist(AddressBook &addresses)
{
    int num = addresses.addresses.size();
    for (int i=0; i<num; ++i)
    {
        char name[30], address[60], phone[15];
        sprintf(name, "Person #%d", i);
        sprintf(address, "%d nowhere avenue", i);
        sprintf(phone, "07886 %d", i);

        typename AddressBook::Person &p = addresses.addresses[name];

        // assert(p.name == name);
        assert(p.address == address);
        assert(p.telephone == phone);
    }
}

template<class AddressBook>
void read_rand_persist(AddressBook &addresses)
{
    int num = addresses.addresses.size();
    for (int i=0; i<num; ++i)
    {
        int target = (i*73)%num;

        char name[30], address[60], phone[15];
        sprintf(name, "Person #%d", target);
        sprintf(address, "%d nowhere avenue", target);
        sprintf(phone, "07886 %d", target);

        typename AddressBook::Person &p = addresses.addresses[name];

        // assert(p.name == name);
        assert(p.address == address);
        assert(p.telephone == phone);

    }
}


template<class AddressBook>
void delete_persist(AddressBook &addresses)
{
    int num = addresses.addresses.size();
    for (int i=0; i<num; ++i)
    {
        char name[30];
        sprintf(name, "Person #%d", i);

        typename AddressBook::Pmap::iterator j=addresses.addresses.find(name);

        assert(j!=addresses.addresses.end());
        addresses.addresses.erase(j);
    }

}


const int ms(int cd)
{
    return 1000*cd/CLOCKS_PER_SEC;
}


int main(int argc, char **argv)
{
    if (argc!=3)
    {
        std::cout << "Usage: [ram|persist] <number>\n";
        return 1;
    }

    int n = atoi(argv[2]);
    time_t t0, t1, t2, t3, t4, t5;

    if (strcmp(argv[1], "persist")==0)
    {
        try
        {
            t0 = clock();
            map_data<izenelib::am::AddressBook> root("bench.map", 0x10000000, create_new|auto_grow);

            if (root)
            {
                t1 = clock();
                create_persist(*root, n);
                t2 = clock();
                read_seq_persist(*root);
                t3 = clock();
                read_rand_persist(*root);
                t4 = clock();
                delete_persist(*root);
                t5 = clock();
            }
            else
            {
                cout << "Failed to map file\n";
                return 2;
            }
        }
        catch (std::bad_alloc)
        {
            cout << "Out of memory\n";
            return 3;
        }
    }
    else if (strcmp(argv[1], "ram")==0)
    {
        t0 = clock();
        std::AddressBook book;

        t1 = clock();
        create_persist(book, n);
        t2 = clock();
        read_seq_persist(book);
        t3 = clock();
        read_rand_persist(book);
        t4 = clock();
        delete_persist(book);
        t5 = clock();
    }
    else
    {
        cout << "Invalid option\n";
        return 1;  // I
    }

    cout << argv[1] << " " << n << ": setup=" << ms(t1-t0) << " create=" << ms(t2-t1) <<
         " seq_read=" << ms(t3-t2) << " rand_read=" << ms(t4-t3) <<
         " delete=" << ms(t5-t4) << endl;

    return 0;
}

