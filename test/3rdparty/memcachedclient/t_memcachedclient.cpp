#include <iostream>
#include <libmemcached/memcached.h>

using namespace std;

int main(int argv, char** argc)
{
    const char *config_string = "--SERVER=localhost";

    memcached_st * memc = memcached(config_string, strlen(config_string));

    // Adding a value to the server
    char *key = "foo";
    char *value = "value";

    memcached_return_t rc= memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

    if (rc != MEMCACHED_SUCCESS)
    {
        cout << "Failed to add value" <<endl;
    }
    else
    {
        cout << "Successed to add value." <<endl;
    }

    memcached_free(memc);
}
