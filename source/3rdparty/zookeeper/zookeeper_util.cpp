#include <zookeeper_util.hpp>
#include <zookeeper_watcher.hpp>

#include <zookeeper.h>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

namespace
{
void watcher_callback(zhandle_t *zk, int type, int state,
                      const char *path, void *ctx)
{
    ZooKeeperWatcher *watcher = (ZooKeeperWatcher*)ctx;
    if (watcher == NULL) return;
    watcher->callback(type, state, string(path));
}
} // namespace

int ZooKeeperUtil::createAndFailSilent(ZooKeeperWatcher* w, const string& path)
{
    zhandle_t* zk = w->getZooKeeper();
    if (zk == NULL || path.empty()) return -1;
    int flag = 0; // persistent
    int r = zoo_create(zk, path.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE,
                       flag, NULL, 0);
    return (r == ZOK || r == ZNODEEXISTS) ? 0 : -1;
}

int ZooKeeperUtil::watchAndCheckExists(ZooKeeperWatcher* w, const string& path)
{
    zhandle_t* zk = w->getZooKeeper();
    if (zk == NULL || path.empty()) return -1;
    Stat stat;
    int r = zoo_wexists(zk, path.c_str(), watcher_callback, w, & stat);
    return (r == ZOK) ? 0 : r;
}

int ZooKeeperUtil::getDataAndWatch(ZooKeeperWatcher* w, const string& path,
                                   vector<char>& data)
{
    zhandle_t* zk = w->getZooKeeper();
    if (zk == NULL || path.empty()) return -1;

    // Kazuki Ohta <kazuki.ohta@gmail.com>
    // BUG: this buffersize is not enough to get the data.
    // but how to decide this?
    int bufsiz = 1024 * 32;
    char buf[bufsiz];
    int len = bufsiz;
    Stat stat;
    int r = zoo_get(w->getZooKeeper(), path.c_str(), NULL,
                    buf, &len, &stat);

    data.clear();
    assert(len <= bufsiz);
    if (len > 0)
    {
        data.resize(len);
        memcpy(&data[0], buf, len);
    }

    cerr << "r=:" << r << ", len=" << len << endl;
    return (r == ZOK) ? 0 : r;
}

