#include <string.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <set>
#include <string>
#include <stdio.h>

#include <libcassandra/connection_manager.h>
#include <libcassandra/cassandra.h>

using namespace std;
using namespace org::apache::cassandra;
using namespace libcassandra;

static const string host("180.153.140.105");
static const string username("admin");
static const string password("~Yeogirl!Yun@Is#Watching$You%");
static int port= 9160;
static size_t pool_size = 16;

int main()
{
    try
    {
        static const string ks_name("drizzle");
        static const string key("sarah");
        static const string col_value("this is data being inserted!");
        static const string col_family("Data");
        static const string sup_col_name("Test");
        static const string col_name("third");

        CassandraConnectionManager::instance()->init(host,port,pool_size);
        boost::shared_ptr<Cassandra> client(new Cassandra());
        client->login(username, password);

        string clus_name= client->getClusterName();
        cout << "cluster name: " << clus_name << endl;

        ColumnParent col_parent;
        col_parent.__set_column_family(col_family);
        col_parent.__set_super_column(sup_col_name);
        SlicePredicate pred;

        /* create keyspace */
        cout << "Create keyspace: " << ks_name << endl;
        KsDef ks_def;
        ks_def.__set_name(ks_name);
        ks_def.__set_strategy_class("SimpleStrategy");
        map<string, string> strategy_options;
        strategy_options["replication_factor"] = "1";
        ks_def.__set_strategy_options(strategy_options);
        client->createKeyspace(ks_def);
        client->setKeyspace(ks_name);

        client->reloadKeyspaces();
        cout << "Current keyspaces are:" << endl;
        {
            const vector<KsDef>& key_out= client->getKeyspaces();
            for (vector<KsDef>::const_iterator it = key_out.begin(); it != key_out.end(); ++it)
            {
                cout << it->name << endl;
            }
        }
        cout << endl;

        /* create standard column family */
        CfDef cf_def;
        cf_def.__set_name(col_family);
        cf_def.__set_column_type("Super");
        cf_def.__set_keyspace(ks_def.name);
        std::map<std::string, std::string> compress_options;
        compress_options["sstable_compression"] = "SnappyCompressor";
        compress_options["chunk_length_kb"] = "64";
        cf_def.__set_compression_options(compress_options);
        client->createColumnFamily(cf_def);
        cout << "Now we have " << client->getCount(key, col_parent, pred) << " column(s) in the super column." << endl << endl;

        /* insert data */
        cout << "Value will be inserted is: " << col_value << endl;
        client->insertColumn(col_value, key, col_family, sup_col_name, col_name);
        cout << endl << "Inserting...." << endl;

        /* retrieve that data */
        cout << "Now we have " << client->getCount(key, col_parent, pred) << " column(s) in the super column." << endl << endl;
        string res;
        client->getColumnValue(res, key, col_family, sup_col_name, col_name);
        cout << "Value in column retrieved is: " << res << endl;

        /* delete data */
        cout << endl << "Deleting...." << endl;
        client->removeColumn(key, col_family, sup_col_name, col_name);
        cout << "Now we have " << client->getCount(key, col_parent, pred) << " column(s) in the super column." << endl << endl;
    }
    catch (const org::apache::cassandra::InvalidRequestException &ire)
    {
        cerr << "Invalid request: " << ire.why << endl;
        return 0; // exit normally
    }
    catch (const ::apache::thrift::TException &ex)
    {
        cerr << "Other error: " << ex.what() << endl;
        return 0; // exit normally
    }

    return 0;
}
