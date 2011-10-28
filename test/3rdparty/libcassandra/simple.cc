#include <string.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <set>
#include <string>
#include <stdio.h>

#include <libcassandra/cassandra_factory.h>
#include <libcassandra/cassandra.h>
#include <libcassandra/column_family_definition.h>
#include <libcassandra/keyspace.h>
#include <libcassandra/keyspace_definition.h>

using namespace std;
using namespace libcassandra;

static string host("127.0.0.1");
static int port= 9160;

int main()
{
    CassandraFactory factory(host, port);
    boost::shared_ptr<Cassandra> client(factory.create());

    string clus_name= client->getClusterName();
    cout << "cluster name: " << clus_name << endl;

    try
    {
        const string ks_name("drizzle");
        const string cl_value("this is data being inserted!");

        /* create keyspace */
        cout << "Create keyspaces: " << ks_name << endl;
        KeyspaceDefinition ks_def;
        ks_def.setName(ks_name);
        client->createKeyspace(ks_def);
        client->setKeyspace(ks_def.getName());

        cout << "Current keyspaces are:" << endl;
        vector<KeyspaceDefinition> key_out= client->getKeyspaces();
        for (vector<KeyspaceDefinition>::iterator it = key_out.begin(); it != key_out.end(); ++it)
        {
            cout << (*it).getName() << endl;
        }

        /* create standard column family */
        ColumnFamilyDefinition cf_def;
        cf_def.setName("Data");
        cf_def.setKeyspaceName(ks_def.getName());
        client->createColumnFamily(cf_def);

        /* insert data */
        client->insertColumn("sarah", "Data", "third", cl_value);
        cout << "Value will be inserting is: " << cl_value << endl;
        /* retrieve that data */
        cout << "Inserting...." << endl;
        string res= client->getColumnValue("sarah", "Data", "third");
        cout << "Value in column retrieved is: " << res << endl;

        /* clear */
        client->removeColumn("sarah", "Data", "", "third");
        client->dropColumnFamily("Data");
        cout << "Drop keyspaces: " << ks_name << endl;
        client->dropKeyspace(ks_name);

        cout << "Current keyspaces are:" << endl;
        key_out= client->getKeyspaces();
        for (vector<KeyspaceDefinition>::iterator it = key_out.begin(); it != key_out.end(); ++it)
        {
            cout << (*it).getName() << endl;
        }
    }
    catch (org::apache::cassandra::InvalidRequestException &ire)
    {
        cout << ire.why << endl;
        return 1;
    }

    return 0;
}
