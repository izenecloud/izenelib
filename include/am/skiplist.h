/****************************************************************************
 * File: skiplist.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * An implementation of a set abstraction backed by a skiplist.  A skiplist
 * is a probabilistic data structure introduced by William Pugh in his paper
 * "Skip Lists: A Probabilistic Alternative to Balanced Trees."  The skiplist
 * can be thought of as a sorted linked list where each node stores pointers
 * to a collection of nodes further down in the list, not just the next node.
 * These pointers are "stacked" on top of one another, with the topmost pointer
 * pointing furthest down in the structure, the second-to-topmost pointer
 * pointing no further than that, etc.  The bottommost level points directly to
 * the next node in the skiplist.  The skiplist structure itself maintains an
 * array of pointers stored in the same fashion.  Searches can then be done by
 * starting at the top of the pointer stack, then advancing to the indicated
 * node if its value is no greater than the key to search for, and otherwise
 * dropping down a level and repeating.
 *
 * The main advantage of a skiplist over a standard self-balancing binary
 * search tree is that the skiplist implemention is significantly easier than
 * most balanced tree implementations.  There are no tree rotations, nor
 * "colors" or "balance factors" to keep track of.  Instead, the balancing
 * comes probabalistically with the choice of the heights of each node.
 * Moreover, the constant factors on skiplist implementations of search,
 * insert, and delete operations are often much lower than the constant factors
 * on typical balanced BSTs.
 *
 * This implementation of the skiplist uses the skiplist to represent an
 * associative array structure like the STL map.  Each entry stores a constant
 * key and mutable value, as well as the forward pointers.  The structure also
 * supports forward iterators, which can read and write entries.  However, in
 * the interests of simplicity, this implementation does not comply with the
 * associative container requirements of the C++ standard; this would require
 * an enormous amount of extra code that could complicate the implementation
 * without necessarily adding anything interesting.
 *
 * This code does contain one optimization which might make it a bit harder
 * to read.  Although any two nodes in a skiplist might support different
 * numbers of pointers, once a node is constructed the number of pointers it
 * stores is fixed.  Rather than having each node store a vector of pointers or
 * a dynamically-allocated array of pointers, I instead override the new and
 * delete operators for nodes so that when a node is constructed on the heap,
 * it is overallocated with space to store the extra pointers.  This saves an
 * indirection to locate the pointer array, since they're bundled directly with
 * the object itself.  This was mostly for my own edification (I've seen this
 * technique used before, but never implemented it myself), and I apologize if
 * it complicates the implementation.
 */
#ifndef IZENELIB_AM_SKIPLIST
#define IZENELIB_AM_SKIPLIST

#include <algorithm>   // For lexicographical_compare, equal, max
#include <functional>  // For less
#include <utility>     // For pair
#include <iterator>    // For iterator
#include <cstring>     // For memset
#include <cstdlib>     // For rand
#include <stdexcept>   // For out_of_range

namespace izenelib{ namespace am{
/**
 * A map-like class backed by a skiplist.
 */
template <typename Key, typename Value, typename Comparator = std::less<Key> >
class Skiplist
{
public:
    /**
     * Constructor: Skiplist(Comparator comp = Comparator());
     * Usage: Skiplist<string, int> mySkiplist;
     * Usage: Skiplist<string, int> mySkiplist(MyComparisonFunction);
     * -------------------------------------------------------------------------
     * Constructs a new, empty skiplist that uses the indicated comparator to
     * compare keys.
     */
    Skiplist(Comparator comp = Comparator());

    /**
     * Destructor: ~Skiplist();
     * Usage: (implicit)
     * -------------------------------------------------------------------------
     * Destroys the skiplist, deallocating all memory allocated internally.
     */
    ~Skiplist();

    /**
     * Copy functions: Skiplist(const Skiplist& other);
     *                 Skiplist& operator= (const Skiplist& other);
     * Usage: Skiplist<string, int> one = two;
     *        one = two;
     * -------------------------------------------------------------------------
     * Makes this skiplist equal to a deep-copy of some other skiplist.
     */
    Skiplist(const Skiplist& other);
    Skiplist& operator= (const Skiplist& other);

    /**
     * Type: iterator
     * Type: const_iterator
     * -------------------------------------------------------------------------
     * A pair of types that can traverse the elements of a skiplist in ascending
     * order.
     */
    class iterator;
    class const_iterator;

    /**
     * std::pair<iterator, bool> insert(const Key& key, const Value& value);
     * Usage: mySkiplist.insert("Skiplist", 137);
     * -------------------------------------------------------------------------
     * Inserts the specified key/value pair into the skiplist.  If an entry with
     * the specified key already existed, this function returns false paired
     * with an iterator to the extant value.  If the entry was inserted
     * successfully, returns true paired with an iterator to the new element.
     */
    std::pair<iterator, bool> insert(const Key& key, const Value& value);

    /**
     * bool erase(const Key& key);
     * Usage: mySkiplist.erase("AVL Tree");
     * -------------------------------------------------------------------------
     * Removes the entry from the skiplist with the specified key, if it exists.
     * Returns whether or not an element was erased.
     */
    bool erase(const Key& key);

    /**
     * iterator find(const Key& key);
     * const_iterator find(const Key& key);
     * Usage: if (mySkiplist.find("Skiplist") != mySkiplist.end()) { ... }
     * -------------------------------------------------------------------------
     * Returns an iterator to the entry in the skiplist with the specified key,
     * or end() as as sentinel if it does not exist.
     */
    iterator find(const Key& key);
    const_iterator find(const Key& key) const;

    /**
     * Value& operator[] (const Key& key);
     * Usage: mySkiplist["skiplist"] = 137;
     * -------------------------------------------------------------------------
     * Returns a reference to the value associated with the specified key in the
     * skiplist.  If the key is not contained in the skiplist, it will be
     * inserted into the skiplist with a default-constructed Entry as its value.
     */
    Value& operator[] (const Key& key);

    /**
     * Value& at(const Key& key);
     * const Value& at(const Key& key) const;
     * Usage: mySkiplist.at("skiplist") = 137;
     * -------------------------------------------------------------------------
     * Returns a reference to the value associated with the specified key,
     * throwing a std::out_of_range exception if the key does not exist in the
     * skiplist.
     */
    Value& at(const Key& key);
    const Value& at(const Key& key) const;

    /**
     * (const_)iterator begin() (const);
     * (const_)iterator end() (const);
     * Usage: for (Skiplist<string, int>::iterator itr = s.begin();
     *             itr != s.end(); ++itr) { ... }
     * -------------------------------------------------------------------------
     * Returns iterators delineating the full contents of the skiplist.  Each
     * iterator acts as a pointer to a std::pair<const Key, Entry>.
     */
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    /**
     * size_t size() const;
     * Usage: cout << "Skiplist contains " << s.size() << " entries." << endl;
     * -------------------------------------------------------------------------
     * Returns the number of elements stored in the skiplist.
     */
    size_t size() const;

    /**
     * bool empty() const;
     * Usage: if (s.empty()) { ... }
     * -------------------------------------------------------------------------
     * Returns whether the skiplist contains no elements.
     */
    bool empty() const;

    /**
     * void swap(Skiplist& other);
     * Usage: one.swap(two);
     * -------------------------------------------------------------------------
     * Exchanges the contents of this skiplist and some other skiplist.
     */
    void swap(Skiplist& other);

private:
    /* A type representing a node in the skiplist.  This node is designed to
     * be allocated on the heap with the number of extra pointers required
     * specified as an argument to operator new.
     */
    struct Node
    {
        std::pair<const Key, Value> mValue; // The actual value stored here
        const size_t mLevel;                // The level of this node

        /* The first of many pointers that may be stored here.  operator new will
         * overallocate this structure to store the remaining pointers.  This
         * MUST be the final data member in this struct.
         */
        Node* mNext[1];

        /* Constructor sets up the value to the specified key/value pair. */
        Node(const Key& key, const Value& value, size_t level);

        /* operator new overallocates storage for the entry. */
        void* operator new (size_t size, size_t numPointers);

        /* Matching operator delete exists in case an exception is thrown. */
        void operator delete (void* memory);
    };

    /* A constant controlling the maximum number of pointers to store in any
     * element.
     */
    static const size_t kMaxLevel = 32; // Enough space to hold 2^32 elements

    /* An array of kMaxLevel pointers to entries in the list, all initially
     * set to NULL.
     */
    Node* mList[kMaxLevel];

    /* The maximum level of any entry actually stored in the list.  Note that
     * this is an inclusive value, and so the pointer referenced by mHighestLevel
     * is a valid pointer.
     */
    size_t mHighestLevel;

    /* The comparator to use when storing elements. */
    Comparator mComp;

    /* The number of elements in the list. */
    size_t mSize;

    /* A utility base class for iterator and const_iterator which actually
     * supplies all of the logic necessary for the two to work together.  The
     * parameters are the derived type, the type of a pointer being visited, and
     * the type of a reference being visited.  This uses the Curiously-Recurring
     * Template Pattern to work correctly.
     */
    template <typename DerivedType, typename Pointer, typename Reference>
    class IteratorBase;
    template <typename DerivedType, typename Pointer, typename Reference>
    friend class IteratorBase;

    /* Make iterator and const_iterator friends as well so they can use the
     * Node type.
     */
    friend class iterator;
    friend class const_iterator;

    /* Utility function to scan over the list and look for an entry with a given
     * key.  This function is marked const even though it returns a mutable
     * pointer into the list because the result is always wrapped up in an
     * iterator or treated correctly by the implementation function that uses
     * it.
     *
     * If the node does not exist, this function returns NULL.
     */
    Node* findNode(const Key& key) const;

    /* A variant of the above function which in addition to returning a pointer
     * to the node in question, returns the pointer stack of all pointers
     * pointing into it at each level.  This could be rolled in to the previous
     * function, but for efficiency reasons I've kept them separate.
     */
    Node* findNodeAndPredecessors(const Key& key, Node** predecessors[]);

    /* A utility function to pick a random level for a node. */
    static size_t chooseRandomLevel();
};

/* Comparison operators for Skiplists. */
template <typename Key, typename Value, typename Comparator>
bool operator<  (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs);
template <typename Key, typename Value, typename Comparator>
bool operator<= (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs);
template <typename Key, typename Value, typename Comparator>
bool operator== (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs);
template <typename Key, typename Value, typename Comparator>
bool operator!= (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs);
template <typename Key, typename Value, typename Comparator>
bool operator>= (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs);
template <typename Key, typename Value, typename Comparator>
bool operator>  (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs);

/* * * * * Implementation Below This Point * * * * */

/* Definition of the IteratorBase type, which is used to provide a common
 * implementation for iterator and const_iterator.
 */
template <typename Key, typename Value, typename Comparator>
template <typename DerivedType, typename Pointer, typename Reference>
class Skiplist<Key, Value, Comparator>::IteratorBase
{
public:
    /* Utility typedef to talk about nodes. */
    typedef typename Skiplist<Key, Value, Comparator>::Node Node;

    /* Advance operators just construct derived type instances of the proper
     * type, then advance them.
     */
    DerivedType& operator++ ()
    {
        mCurr = mCurr->mNext[0];

        /* Downcast to our actual type. */
        return static_cast<DerivedType&>(*this);
    }
    const DerivedType operator++ (int)
    {
        /* Copy our current value by downcasting to our real type. */
        DerivedType result = static_cast<DerivedType&>(*this);

        /* Advance to the next element. */
        ++*this;

        /* Hand back the cached value. */
        return result;
    }

    /* Equality and disequality operators are parameterized - we'll allow anyone
     * whose type is IteratorBase to compare with us.  This means that we can
     * compare both iterator and const_iterator against one another.
     */
    template <typename DerivedType2, typename Pointer2, typename Reference2>
    bool operator== (const IteratorBase<DerivedType2, Pointer2, Reference2>& rhs)
    {
        /* Just check the underlying pointer, which (fortunately!) is of the same
         * type.
         */
        return mCurr == rhs.mCurr;
    }
    template <typename DerivedType2, typename Pointer2, typename Reference2>
    bool operator!= (const IteratorBase<DerivedType2, Pointer2, Reference2>& rhs)
    {
        /* We are disequal if equality returns false. */
        return !(*this == rhs);
    }

    /* Pointer dereference operator hands back a reference. */
    Reference operator* () const
    {
        return mCurr->mValue;
    }

    /* Arrow operator returns a pointer. */
    Pointer operator-> () const
    {
        /* Use the standard "&**this" trick to dereference this object and return
         * a pointer to the referenced value.
         */
        return &**this;
    }

protected:
    /* Where we are in the list. */
    Node* mCurr;

    /* In order for equality comparisons to work correctly, all IteratorBases
     * must be friends of one another.
     */
    template <typename Derived2, typename Pointer2, typename Reference2>
    friend class IteratorBase;

    /* Constructor sets up the skiplist pointer appropriately. */
    IteratorBase(Node* curr = NULL) : mCurr(curr)
    {
        // Handled in initializer list
    }
};

/* iterator and const_iterator implementations work by deriving off of
 * IteratorBase, passing in parameters that make all the operators work.
 * Additionally, we inherit from std::iterator to import all the necessary
 * typedefs to qualify as an iterator.
 */
template <typename Key, typename Value, typename Comparator>
class Skiplist<Key, Value, Comparator>::iterator:
            public std::iterator< std::forward_iterator_tag,
            std::pair<const Key, Value> >,
            public IteratorBase<iterator,                       // Our type
            std::pair<const Key, Value>*,   // Reference type
            std::pair<const Key, Value>&>   // Pointer type
{
public:
    /* Default constructor forwards NULL to base implicity. */
    iterator()
    {
        // Nothing to do here.
    }

    /* All major operations inherited from the base type. */

private:
    /* Constructor for creating an iterator out of a raw node just forwards this
     * argument to the base type.  This line is absolutely awful because the
     * type of the base is so complex.
     */
    iterator(typename Skiplist<Key, Value, Comparator>::Node* node) :
            IteratorBase<iterator,
            std::pair<const Key, Value>*,
            std::pair<const Key, Value>&>(node)
    {
        // Handled by initializer list
    }

    /* Make the Skiplist a friend so it can call this constructor. */
    friend class Skiplist;

    /* Make const_iterator a friend so it can steal the pointer when doing an
     * iterator-to-const_iterator conversion.
     */
    friend class const_iterator;
};

/* Same as above, but with const added in. */
template <typename Key, typename Value, typename Comparator>
class Skiplist<Key, Value, Comparator>::const_iterator:
            public std::iterator< std::forward_iterator_tag,
            const std::pair<const Key, Value> >,
            public IteratorBase<const_iterator,                       // Our type
            const std::pair<const Key, Value>*,   // Reference type
            const std::pair<const Key, Value>&>   // Pointer type
{
public:
    /* Default constructor forwards NULL to base implicity. */
    const_iterator()
    {
        // Nothing to do here.
    }

    /* Conversion constructor from the iterator type initializes the base class
     * as a copy of the iterator's base fields.
     */
    const_iterator(iterator itr) :
            IteratorBase<const_iterator,
            const std::pair<const Key, Value>*,
            const std::pair<const Key, Value>&>(itr.mCurr)
    {
        // Handled in initializer list.
    }

    /* All major operations inherited from the base type. */

private:
    /* See iterator implementation for details about what this does. */
    const_iterator(typename Skiplist<Key, Value, Comparator>::Node* node) :
            IteratorBase<const_iterator,
            const std::pair<const Key, Value>*,
            const std::pair<const Key, Value>&>(node)
    {
        // Handled by initializer list
    }

    /* Make the Skiplist a friend so it can call this constructor. */
    friend class Skiplist;
};

/**** Skiplist::Node Implementation. ****/

/* Constructor initializes the key/value pair using its arguments. */
template <typename Key, typename Value, typename Comparator>
Skiplist<Key, Value, Comparator>::Node::Node(const Key& key, const Value& value, size_t level)
        : mValue(key, value), mLevel(level)
{
    // Handled in initializer list
}

/* operator new overallocates space so that there are sufficiently many
 * pointers available off the end of the struct.
 */
template <typename Key, typename Value, typename Comparator>
void* Skiplist<Key, Value, Comparator>::Node::operator new (size_t size, size_t numPointers)
{
    /* The Node itself contains one pointer, so we need to allocate space for
     * numPointers - 1 extra pointers.
     */
    const size_t spaceNeeded = size + (numPointers - 1) * sizeof(Node*);
    void* result = ::operator new(spaceNeeded);

    /* Zero out the memory; we want to ensure that the pointers are zeroed
     * before continuing.
     */
    std::memset(result, 0, spaceNeeded);

    return result;
}

/* operator delete implementation exists in case the constructor throws an
 * exception.  This should never happen, but we should put this here anyway
 * in case a future version ends up throwing.
 */
template <typename Key, typename Value, typename Comparator>
void Skiplist<Key, Value, Comparator>::Node::operator delete(void* memory)
{
    /* This can be handled using the default operator delete implementation. */
    ::operator delete(memory);
}

/**** Skiplist Implementation ****/

/* Constructor zeros out relevant fields. */
template <typename Key, typename Value, typename Comparator>
Skiplist<Key, Value, Comparator>::Skiplist(Comparator comp) : mComp(comp)
{
    /* Set all of the node pointers to NULL. */
    std::memset(mList, 0, sizeof(mList));

    /* Our highest level is zero, since none of the pointers are valid. */
    mHighestLevel = 0;

    /* Our size is zero, since we currently have no elements. */
    mSize = 0;
}

/* Destructor walks across the skiplist, deleting elements. */
template <typename Key, typename Value, typename Comparator>
Skiplist<Key, Value, Comparator>::~Skiplist()
{
    /* Walk across the bottom of the list. */
    Node* curr = mList[0];
    while (curr != NULL)
    {
        /* Cache where to go next, since we're about to destroy our last reference
         * to it.
         */
        Node* next = curr->mNext[0];

        /* Free memory, then advance to the next location. */
        delete curr;
        curr = next;
    }
}

/* begin hands back a (const_)iterator initialized to the head of the list. */
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::iterator
Skiplist<Key, Value, Comparator>::begin()
{
    return iterator(mList[0]); // Scan bottom row
}
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::const_iterator
Skiplist<Key, Value, Comparator>::begin() const
{
    return const_iterator(mList[0]); // Scan bottom row
}

/* end hands back a (const_)iterator initialized to NULL, which comes one step
 * past the end of the list.
 */
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::iterator
Skiplist<Key, Value, Comparator>::end()
{
    return iterator(NULL);
}
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::const_iterator
Skiplist<Key, Value, Comparator>::end() const
{
    return const_iterator(NULL);
}

/* We cache the size to simplify this implementation. */
template <typename Key, typename Value, typename Comparator>
size_t Skiplist<Key, Value, Comparator>::size() const
{
    return mSize;
}

/* Checking for emptiness just checks whether the stored size is zero. */
template <typename Key, typename Value, typename Comparator>
bool Skiplist<Key, Value, Comparator>::empty() const
{
    return size() == 0;
}

/* Checking whether an element exists involves scanning over the skiplist
 * level-by-level looking for the key.
 */
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::Node*
Skiplist<Key, Value, Comparator>::findNode(const Key& key) const
{
    /* At each step, we need to maintain an array of pointers containing
     * possible places to look.  This is initially the skiplist's own master
     * list of pointers.
     */
    Node* const* table = mList;

    /* The scan works as follows.  We start at the topmost level and continue
     * advancing along it as long as the next node exists and has a key that is
     * less than the key in question.  We then drop one level down and repeat
     * this process until we finish scanning the bottom level.
     *
     * During this scan, we use integers to encode the level so that if we drop
     * below the first level, we don't integer-overflow up to some outrageously
     * high level.
     */
    for (int level = int(mHighestLevel); level >= 0; --level)
        while (table[level] && mComp(table[level]->mValue.first, key))
            table = table[level]->mNext;

    /* Finally, once we've hit the bottom, we're looking at a pointer stack that
     * comes right before the node we want (if it exists) or some other random
     * node if the entry doesn't exist.  Check which of these two cases holds.
     */
    if (table[0] &&                            // Entry exists
            !mComp(key, table[0]->mValue.first) && // ... and isn't less than key
            !mComp(table[0]->mValue.first, key))   // ... and isn't greater than key
        return table[0];

    /* Otherwise we didn't find it. */
    return NULL;
}

/* Both versions of find work by calling findNode and then wrapping up the
 * result.
 */
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::iterator
Skiplist<Key, Value, Comparator>::find(const Key& key)
{
    return iterator(findNode(key));
}
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::const_iterator
Skiplist<Key, Value, Comparator>::find(const Key& key) const
{
    return const_iterator(findNode(key));
}

/* Checking whether an element exists involves scanning over the skiplist
 * level-by-level looking for the key.
 */
template <typename Key, typename Value, typename Comparator>
typename Skiplist<Key, Value, Comparator>::Node*
Skiplist<Key, Value, Comparator>::findNodeAndPredecessors(const Key& key,
        Node** predecessors[])
{
    /* At each step, we need to maintain an array of pointers containing
     * possible places to look.  This is initially the skiplist's own master
     * list of pointers.
     */
    Node** table = mList;

    /* The scan works as follows.  We start at the topmost level and continue
     * advancing along it as long as the next node exists and has a key that is
     * less than the key in question.  We then drop one level down and repeat
     * this process until we hit the bottom level.  At the end of each scan,
     * we record the last pointer we found.
     */
    for (int level = int(mHighestLevel); level >= 0; --level)
    {
        /* Walk forward as far as possible. */
        while (table[level] && mComp(table[level]->mValue.first, key))
            table = table[level]->mNext;

        /* Record this entry. */
        predecessors[level] = &table[level];
    }

    /* Finally, once we've hit the bottom, we're looking at a pointer stack that
     * comes right before the node we want (if it exists) or some other random
     * node if the entry doesn't exist.  Check which of these two cases holds.
     */
    if (table[0] &&                            // Entry exists
            !mComp(key, table[0]->mValue.first) && // ... and isn't less than key
            !mComp(table[0]->mValue.first, key))   // ... and isn't greater than key
        return table[0];

    /* Otherwise we didn't find it. */
    return NULL;
}

/* Picks a random level for a node by advancing upward and upward with
 * uniform probability.
 */
template <typename Key, typename Value, typename Comparator>
size_t Skiplist<Key, Value, Comparator>::chooseRandomLevel()
{
    /* We advance to a new level with probability 1/4 at each point, as suggested
     * by the original paper.  To do this with a minimum of floating-point
     * computations, we compute RAND_MAX / 4 and then go up a level every time
     * rand() is no greater than this value.
     */
    static const int kLevelProbability = RAND_MAX / 4;

    /* In the worst case, we have one level of pointers. */
    size_t result = 1;

    /* Loop while we keep choosing to promote and while the level is no larger
     * than the max level.
     */
    while (rand() < kLevelProbability && result < kMaxLevel)
        ++result;

    return result;
}

/* Insertion into the skiplist works by building up the predecessors, then
 * inserting the new node with some arbitrary height at the indicated spot.
 */
template <typename Key, typename Value, typename Comparator>
std::pair<typename Skiplist<Key, Value, Comparator>::iterator, bool>
Skiplist<Key, Value, Comparator>::insert(const Key& key, const Value& value)
{
    /* Begin by calling the find predecessors function to determine what comes
     * right before this node.
     */
    Node** predecessors[kMaxLevel];
    Node* entry = findNodeAndPredecessors(key, predecessors);

    /* If this node already exists, hand back an iterator to it, marking that
     * we didn't insert anything.
     */
    if (entry != NULL)
        return std::make_pair(iterator(entry), false);

    /* Otherwise, we need to actually insert the node here.  Begin by picking
     * a random level for it.
     */
    const size_t level = chooseRandomLevel();

    /* Create the node object to hold the key/value pair.  We pass the level as
     * an argument to new to ensure space exists for the pointers.
     */
    Node* node = new (level) Node(key, value, level);

    /* To splice this node into the list, we'll make all of its outgoing
     * pointers on each of its levels point to the location the predecessor used
     * to be pointing.  We'll also change the predecessors to point to this
     * node instead of where they were pointing.
     */
    for (size_t i = 0; i < level; ++i)
    {
        /* If this level exceeds the maximum level, then the predecessor is the
         * root noot and not whatever garbage coincidentally happened to be in
         * the array.
         */
        if (i > mHighestLevel)
        {
            node->mNext[i] = NULL;
            mList[i] = node;
        }
        /* Otherwise, the predecessor is what was stored in the table. */
        else
        {
            node->mNext[i] = *predecessors[i];
            *predecessors[i] = node;
        }
    }

    /* Update the max level stored in the list in case this is the new
     * largest element.
     */
    mHighestLevel = std::max(mHighestLevel, level);

    /* Increase the size, since we just added an entry. */
    ++mSize;

    /* Return an iterator to the new element, paired with true because something
     * was added.
     */
    return std::make_pair(iterator(node), true);
}

/* The const version of at uses findNode to locate the element, then complains
 * if nothing was found.
 */
template <typename Key, typename Value, typename Comparator>
const Value& Skiplist<Key, Value, Comparator>::at(const Key& key) const
{
    /* Look up the node and return its value if found. */
    if (Node* node = findNode(key))
        return node->mValue.second;
    throw std::out_of_range("Key does not exist in skiplist.");
}

/* Non-const version implemented in terms of const version using the
 * const_cast/static_cast trick.
 */
template <typename Key, typename Value, typename Comparator>
Value& Skiplist<Key, Value, Comparator>::at(const Key& key)
{
    return const_cast<Value&>(static_cast<const Skiplist*>(this)->at(key));
}

/* operator[] implemented by inserting a dummy element with insert, then using
 * the returned iterator to extract the value.
 */
template <typename Key, typename Value, typename Comparator>
Value& Skiplist<Key, Value, Comparator>::operator[] (const Key& key)
{
    return insert(key, Value()).first->second;
}

/* Erasing an element works by locating its predecessors, then wiring them
 * around the element to be deleted.
 */
template <typename Key, typename Value, typename Comparator>
bool Skiplist<Key, Value, Comparator>::erase(const Key& key)
{
    /* Begin by calling the find predecessors function to determine what comes
     * right before this node.
     */
    Node** predecessors[kMaxLevel];
    Node* entry = findNodeAndPredecessors(key, predecessors);

    /* If the node doesn't exist, we don't have any work to do. */
    if (entry == NULL)
        return false;

    /* We now need to take this node out of the list.  We do this by walking
     * over its predecessors and rewiring them to point to the element that
     * comes right after the element.
     */
    for (size_t i = 0; i < entry->mLevel; ++i)
        *predecessors[i] = entry->mNext[i];

    /* Actually delete the node to ensure that the memory isn't leaked. */
    delete entry;

    /* Determine if the level of the list needs to be updated.  We do this by
     * marching downward across the master pointer table, decrementing the count
     * every time we find a null entry.
     */
    while (mHighestLevel > 0 && mList[mHighestLevel] == NULL)
        --mHighestLevel;

    /* Decrement the size, since we just lost an entry. */
    --mSize;

    /* Hand back true, since something was removed. */
    return true;
}

/* Copy constructor deep-copies the other list by inserting all of the other
 * list's elements into this list one at a time.  This is by no means the most
 * efficient way to accomplish this, but it's simple to implement and avoids
 * all sorts of awful pointer wrangling.
 */
template <typename Key, typename Value, typename Comparator>
Skiplist<Key, Value, Comparator>::Skiplist(const Skiplist& other) : mComp(other.mComp)
{
    /* Clear out the pointers, size fields, etc. */
    std::memset(mList, 0, sizeof(mList));
    mHighestLevel = mSize = 0;

    /* Add all of the elements from the other list to this list. */
    for (const_iterator itr = other.begin(); itr != other.end(); ++itr)
        insert(itr->first, itr->second);
}

/* Assignment operator implemented using the copy-and-swap approach. */
template <typename Key, typename Value, typename Comparator>
Skiplist<Key, Value, Comparator>&
Skiplist<Key, Value, Comparator>::operator= (const Skiplist& other)
{
    Skiplist clone = other;
    clone.swap(*this);
    return *this;
}

/* swap just uses the standard swap function to exchange the contents of this
 * skiplist and the other skiplist.
 */
template <typename Key, typename Value, typename Comparator>
void Skiplist<Key, Value, Comparator>::swap(Skiplist& other)
{
    /* Swap pointers. */
    for (size_t i = 0; i < kMaxLevel; ++i)
        std::swap(mList[i], other.mList[i]);

    /* Swap sizes and heights. */
    std::swap(mSize, other.mSize);
    std::swap(mHighestLevel, other.mHighestLevel);

    /* Exchange comparators so we don't end up using the wrong comparison
     * functions.
     */
    std::swap(mComp, other.mComp);
}

/* Comparison operators == and < use the standard STL algorithms. */
template <typename Key, typename Value, typename Comparator>
bool operator<  (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}
template <typename Key, typename Value, typename Comparator>
bool operator== (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs)
{
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(),
            rhs.begin());
}

/* Remaining comparisons implemented in terms of the above comparisons. */
template <typename Key, typename Value, typename Comparator>
bool operator<= (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs)
{
    /* x <= y   iff !(x > y)   iff !(y < x) */
    return !(rhs < lhs);
}
template <typename Key, typename Value, typename Comparator>
bool operator!= (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs)
{
    return !(lhs == rhs);
}
template <typename Key, typename Value, typename Comparator>
bool operator>= (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs)
{
    /* x >= y   iff !(x < y) */
    return !(lhs < rhs);
}
template <typename Key, typename Value, typename Comparator>
bool operator>  (const Skiplist<Key, Value, Comparator>& lhs,
                 const Skiplist<Key, Value, Comparator>& rhs)
{
    /* x > y iff y < x */
    return rhs < lhs;
}

}}
#endif