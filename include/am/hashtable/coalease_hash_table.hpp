#ifndef coalesced_hashing_based_hash_table_h
#define coalesced_hashing_based_hash_table_h

////////////////////////////////////////////////////////////////////////////
// Class CoaleaseHashTable implements a hash table using coaleased hashing
// scheme\cite{Algorithms}; i.e. collisions are resolved by chaining
// within the hash array.
////////////////////////////////////////////////////////////////////////////

#include "ordering.hpp"

#include<types.h>

NS_IZENELIB_AM_BEGIN

////////////////////////////////////////////////////////////////////////////
//  Class |CoaleaseHashTable| is parameterized with the class of the
//  key and the class of the value.  Furthermore, the functions
//      unsigned int hash(const K&);          and
//      bool     equal(const K&, const K&);
//  must be defined by the client that uses this template.
////////////////////////////////////////////////////////////////////////////
template <class K, class V>
class CoaleaseHashTable
{

    K *     keys;           // the array of keys
    V *     values;         // the array of values
    int *   chain;          // collision chain + status
    int     table_size;     // size of the array
    int     elem_count;     // number of elements
    double  max_load_ratio; // maximum load ratio (> 0 && < 1)
    double  growth_ratio;   // amount to grow when expanding
    int     max_load;       // maximum elements before resizing
    int     next_free;      // next free cell to use

    ////////////////////////////////////////////////////////////////////
    // Implementation note: the array |chain| performs two functions:
    // one is to mark the current entry as used, and secondly,
    // to provide a link to the next collided key.  Interpretation
    // is as follows:
    //     (1) chain[i] == 0 means the entry is unused
    //     (2) chain[i] == -1 means the entry is used
    //     (3) chain[i] > 0 means the entry is used; furthermore,
    //         the next entry within the chain is
    //            chain[i]-1;
    ////////////////////////////////////////////////////////////////////

public:
    ////////////////////////////////////////////////////////////////////
    //  Constructor and destructor
    ////////////////////////////////////////////////////////////////////
    CoaleaseHashTable( int initial_size  = 32,
                double max_load_ratio = 0.80,
                double growth_ratio   = 2.0
              );
    ~CoaleaseHashTable();

    ////////////////////////////////////////////////////////////////////
    //  Assignment
    ////////////////////////////////////////////////////////////////////
    void operator = (const CoaleaseHashTable&);

    ////////////////////////////////////////////////////////////////////
    //  Selectors
    ////////////////////////////////////////////////////////////////////
    inline int  capacity() const
    {
        return table_size;    // current capacity
    }
    inline int  size()     const
    {
        return elem_count;    // number of elements
    }
    inline bool is_empty() const
    {
        return elem_count == 0;
    }
    inline bool is_full()  const
    {
        return elem_count == table_size;
    }

    inline double occupy() const
    {
        return (double)elem_count/table_size;
    }

    inline bool contains(const K& k) const
    {
        return lookup(k) != 0;
    }
    inline const V& operator [] (const K& k) const
    {
        return value(lookup(k));
    }
    inline V& operator [] (const K& k)
    {
        return value(lookup(k));
    }

    ////////////////////////////////////////////////////////////////////
    // Insertion and deletion.
    ////////////////////////////////////////////////////////////////////
    void clear();                      // clears out the hash table
    int   lookup(const K&) const;       // lookup entry by key
    int   insert(const K&, const V&);   // insert a new entry
    bool remove(const K&);             // remove an old entry

    ////////////////////////////////////////////////////////////////////
    // Iteration:
    //    first()  start the iteration
    //    next()   get index to the next element; or 0 if none
    //    key()    get the key on index
    //    value()  get the value on index
    ////////////////////////////////////////////////////////////////////
    inline int first() const
    {
        return find_next(0);
    }
    inline int next(int i) const
    {
        return find_next((int)i);
    }
    inline const K& key(int i)   const
    {
        return keys[(int)i-1];
    }
    inline const V& value(int i) const
    {
        return values[(int)i-1];
    }
    inline V& value(int i)
    {
        return values[(int)i-1];
    }

    ////////////////////////////////////////////////////////////////////
    //  Resizing
    ////////////////////////////////////////////////////////////////////
    void resize(int new_size = 0);

private:
    ////////////////////////////////////////////////////////////////////
    //  Addition implementation methods
    ////////////////////////////////////////////////////////////////////
    inline int find_next(int i) const;  // locate the next used entry
};

//////////////////////////////////////////////////////////////////////////
//  Implementation of the template methods
//  Note: we actual do not keep a free list per se, as in the textbook
//  version.  Instead, when a new cell is needed we'll just scan for one
//  linearly.  Although asymtotically the time complexity for insertion
//  will increase(especially with heavily loaded tables), I think this
//  is still usable in practice since the loop is tight.   Furthermore,
//  keeping a free list requires the ability to delete any element
//  from the list, which is hard to do efficiently unless a doubly linked
//  list is used(which would increase the memory requirement).
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//  Locate the next used cell; called by the iterator functions
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
inline int CoaleaseHashTable<K,V>::find_next(register int i) const
{
    while (i < table_size) if (chain[i++]) return i;
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//  Create a new table.
//  Implementation note: each end of each chain of the buckets are
//  linked to the next.  This makes it possible to find the next entry
//  during iteration quickly.
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
CoaleaseHashTable<K,V>::CoaleaseHashTable
(int size, double maximum_load_ratio, double growth)
        : keys(new K [size]), values(new V [size]),
        chain(new int [size]),
        table_size(size)
{
    clear();
    if (maximum_load_ratio > 1.0 || maximum_load_ratio <= 0.1)
        max_load_ratio = 1.0;
    else
        max_load_ratio = maximum_load_ratio;
    if (growth <= 1.2 || growth >= 5.0) growth_ratio = 2.0;
    else growth_ratio = growth;
    max_load = (int)(max_load_ratio * size);
}

//////////////////////////////////////////////////////////////////////////
//  Destroy a table
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
CoaleaseHashTable<K,V>::~CoaleaseHashTable()
{
    delete [] keys;
    delete [] values;
    delete [] chain;
}

//////////////////////////////////////////////////////////////////////////
//  Assignment
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
void CoaleaseHashTable<K,V>::operator = (const CoaleaseHashTable<K,V>& t)
{
    if (this != &t)
    {
        delete [] keys;
        delete [] values;
        delete [] chain;
        elem_count = t.elem_count;
        table_size = t.table_size;
        keys      = new K   [table_size];
        values    = new V   [table_size];
        chain     = new int [table_size];
        next_free = t.next_free;
        for (int i = 0; i < table_size; i++)
        {
            if (chain[i] = t.chain[i])
            {
                keys[i] = t.keys[i];
                values[i] = t.values[i];
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//  Clear a table:  We'll mark all entries as unused.
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
void CoaleaseHashTable<K,V>::clear()
{
    next_free = 0;
    elem_count = 0;
    for (int i = table_size - 1; i >= 0; i--) chain[i] = 0;
}

//////////////////////////////////////////////////////////////////////////
//  Lookup an entry by key; if the entry is not found, return 0.
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
int CoaleaseHashTable<K,V>::lookup(const K& key) const
{
    register unsigned int i = hash(key) % table_size;
    for (;;)
    {
        int s = chain[i];
        if (s == 0) return 0;                 // empty cell located
        if (equal(key,keys[i])) return (i+1); // found entry
        if (s == -1) return 0;                // end of chain located
        i = s-1;                                  // try next element
    }
}

//////////////////////////////////////////////////////////////////////////
//  Insert a new entry; there are two different cases of behavior:
//  (1) If the key doesn't already exists, new key/value pair will be
//      inserted into the table.
//  (2) If the key already exists, then the old value will be overwritten
//      by the new value.
//  Also, if the number of elements have exceeded the maximum load,
//  the table will be automatically resized.
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
int CoaleaseHashTable<K,V>::insert(const K& key, const V& value)
{
    /////////////////////////////////////////////////////////////////////
    // Make sure we have at least one unused cell.
    /////////////////////////////////////////////////////////////////////
    if (elem_count >= max_load) resize();
    int i = hash(key) % table_size;

    for (;;)
    {
        int s = chain[i];
        if (s == 0) break;
        if (equal(key,keys[i]))
        {
            values[i] = value;
            return (i+1);
        }
        if (s == -1)
        {
            ///////////////////////////////////////////////////////////////
            // Got to the end of a chain, we'll find a free cell to use.
            ///////////////////////////////////////////////////////////////
            s = i;
            for (i = next_free; chain[i]; ) if (++i == table_size) i = 0;
            next_free = i;
            chain[s] = i+1;
            break;
        }
        i = s-1;
    }
    keys[i] = key;
    values[i] = value;
    elem_count++;
    chain[i] = -1;
    return (i+1);
}

//////////////////////////////////////////////////////////////////////////
//  Resizing the hash table.  All entries are completed rehashed.
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
void CoaleaseHashTable<K,V>::resize(int new_size)
{
    if (new_size <= elem_count)
        new_size = (int)(table_size * growth_ratio);

    int * new_chain  = new int [ new_size ];
    K   * new_keys   = new K   [ new_size ];
    V   * new_values = new V   [ new_size ];

    //////////////////////////////////////////////////////////////////
    //  Rehash all used cells one by one.  Notice that since all keys
    //  are unique, we don't have to do any comparison.
    //////////////////////////////////////////////////////////////////
    int i;
    for (i = new_size - 1; i >= 0; i--) new_chain[i] = 0;
    next_free = 0;
    for (i = 0; i < table_size; i++)
    {
        if (chain[i])
        {
            int j = hash(keys[i]) % new_size;
            for (;;)
            {
                int s = new_chain[j];
                if (s == 0) break;
                if (s == -1)
                {
                    s = j;
                    for (j = next_free; new_chain[j]; ) if (++j = new_size) j = 0;
                    next_free = j;
                    new_chain[s] = j+1;
                    break;
                }
                j = s-1;
            }
            new_chain[j] = -1;
            new_keys[j] = keys[i];
            new_values[j] = values[i];
        }
    }
    delete [] keys;
    delete [] values;
    delete [] chain;
    keys = new_keys;
    values = new_values;
    chain = new_chain;
    table_size = new_size;
    max_load = (int)(max_load_ratio * table_size);
}

//////////////////////////////////////////////////////////////////////////
//  Remove an entry from the table; there are two different cases:
//  (1) If the key exists within the table, the key/value pair will be
//      removed; otherwise
//  (2) The table will be unaltered.
//      If the removal operation successfully deletes the entry,
//      we'll also return true to the client.
//////////////////////////////////////////////////////////////////////////
template <class K, class V>
bool CoaleaseHashTable<K,V>::remove(const K& key)
{
    unsigned int i = hash(key) % table_size;
    int last = -1;
    for (;;)
    {
        int s = chain[i];
        if (s == 0) return false;
        if (equal(key,keys[i]))
        {
            if (last >= 0) chain[last] = chain[s-1];
            elem_count--;
            chain[i] = 0;
            next_free = i;
            return true;
        }
        if (s == -1) return false;
        last = i;
        i = s-1;
    }
}

NS_IZENELIB_AM_END

#endif
