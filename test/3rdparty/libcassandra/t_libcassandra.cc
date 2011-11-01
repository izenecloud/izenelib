#include <string.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <set>
#include <string>
#include <stdio.h>

#include <libcassandra/connection_manager.h>
#include <libcassandra/cassandra.h>
#include <libcassandra/column_family_definition.h>
#include <libcassandra/keyspace.h>
#include <libcassandra/keyspace_definition.h>

using namespace std;
using namespace org::apache::cassandra;
using namespace libcassandra;

static string host("127.0.0.1");
static int port= 9160;
static size_t pool_size = 16;
int main()
{
    CassandraConnectionManager::instance()->init(host,port,pool_size);
    boost::shared_ptr<Cassandra> client(new Cassandra());

    string clus_name= client->getClusterName();
    cout << "cluster name: " << clus_name << endl;

    try
    {
        const string ks_name("___drizzle___");
        const string key("sarah");
        const string col_value("this is data being inserted!");
        const string col_family("Data");
        const string col_name("third");
        ColumnParent col_parent;
        col_parent.__set_column_family(col_family);
        SlicePredicate pred;

        /* create keyspace */
        cout << "Create keyspace: " << ks_name << endl;
        KeyspaceDefinition ks_def;
        ks_def.setName(ks_name);
        if (client->findKeyspace(ks_name))
        {
            client->dropKeyspace(ks_name);
        }
        client->createKeyspace(ks_def);
        client->setKeyspace(ks_def.getName());

        cout << "Current keyspaces are:" << endl;
        {
            const map<string, KeyspaceDefinition>& key_out= client->getKeyspaces();
            for (map<string, KeyspaceDefinition>::const_iterator it = key_out.begin(); it != key_out.end(); ++it)
            {
                cout << it->first << endl;
            }
        }
        cout << endl;

        /* create standard column family */
        ColumnFamilyDefinition cf_def;
        cf_def.setName(col_family);
        cf_def.setKeyspaceName(ks_def.getName());
        client->createColumnFamily(cf_def);
        cout << "Now we have " << client->getCount(key, col_parent, pred) << " column(s) in the column family." << endl << endl;

        /* insert data */
        cout << "Value will be inserted is: " << col_value << endl;
        client->insertColumn(col_value, key, col_family, col_name);
        cout << endl << "Inserting...." << endl;

        /* retrieve that data */
        cout << "Now we have " << client->getCount(key, col_parent, pred) << " column(s) in the column family." << endl << endl;
        string res;
        client->getColumnValue(res, key, col_family, col_name);
        cout << "Value in column retrieved is: " << res << endl;

        /* delete data */
        cout << endl << "Deleting...." << endl;
        client->removeColumn(key, col_family, col_name);
        cout << "Now we have " << client->getCount(key, col_parent, pred) << " column(s) in the column family." << endl << endl;

        /* drop column family and keyspace */
        client->dropColumnFamily(col_family);
        cout << "Drop keyspace: " << ks_name << endl;
        client->dropKeyspace(ks_name);

        cout << "Current keyspaces are:" << endl;
        {
            const map<string, KeyspaceDefinition>& key_out= client->getKeyspaces();
            for (map<string, KeyspaceDefinition>::const_iterator it = key_out.begin(); it != key_out.end(); ++it)
            {
                cout << it->first << endl;
            }
        }
    }
    catch (org::apache::cassandra::InvalidRequestException &ire)
    {
        cout << ire.why << endl;
        return 1;
    }

    return 0;
}
