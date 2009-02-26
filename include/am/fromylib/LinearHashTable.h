/**
 * @file  LinearHashTable.h
 * @brief The header file of definition of linear hashing table based on memory.
 */

#ifndef LinearHashTable_h
#define LinearHashTable_h 1

#include "ylib_basics.h"
//#include "lht_trait.h"

NS_IZENELIB_AM_BEGIN

/*
 * @brief LHTElem is a wrapper class for DataType and provide next elem
 * for List like behaviour within the LinearHashTable.
 */
template <typename DataType> class LHTElem {
	template <typename D> friend class Segment;

	template <typename KeyType, typename D, typename LockType> friend class LinearHashTable;

private:
	DataType data;
	LHTElem<DataType>* next;
	LHTElem(const LHTElem& source) :
		data(source.data), next(source.next) {
	}
	LHTElem(const DataType& d) :
		data(d), next(NULL) {
	}
	LHTElem& operator=(const LHTElem& source) {
		if (this != &source) {
			data= source.data;
			next = source.next;
		}
		return *this;
	}
	~LHTElem() {
	} // next items are deleted in the Segment typename.
};

/*
 * @brief Segment is used to support dynamic growth/shrink property of
 * LinearHashTable typename.
 */
template <typename DataType> class Segment {
	template <typename KeyType, typename D, typename LockType> friend class LinearHashTable;

private:
	LHTElem<DataType> *seg[SEGMENT_SIZE];

	Segment() {
		for (int i = 0; i < SEGMENT_SIZE; i++)
			seg[i] = NULL;
	}

	~Segment() {
		for (int i = 0; i < SEGMENT_SIZE; i++) {
			if (seg[i]) {
				LHTElem<DataType> *curr = seg[i];
				do {
					// delete all the LHTElem objects following its next link.
					LHTElem<DataType> *next = curr->next;
					delete curr;
					curr = next;
				} while (curr);
			}
		}
	}
};

/**
 *
 * @brief LinearHashTable is based on Per-Ake Larson's work, Dynamic  Hash Tables.
 *
 *  It is the fastest hash table that I know and have implemented.
 *	               There are several parameters to tune for faster performance.
 *				   The min/max load factor is vital in the tuning.
 *				   The segment and directory size could be important factors but
 *				   have not been experimented.
 *
 *  One idea is to expedite the growth/shrink process by expanding
 *	                or contracting more than one buckets. It is not clear yet
 *					whether such a move would be beneficial.
 *
 *
 */
template <typename KeyType, typename ValueType, typename LockType=NullLock > class LinearHashTable :
	 public AccessMethod<KeyType, ValueType, LockType> {
	typedef DataType<KeyType, ValueType> DataType;

	int p; // next bucket to be split
	int maxp; // upper bound on p during this expansion
	int keycount; // number of records in the table
	int currentsize; // current number of buckets
	double minloadfctr, maxloadfctr; // lower and upper bound on the load factor
	Segment<DataType>* directory[DIRECTORY_SIZE];
	LockType lock; // for thread-safe

protected:
	int hash(const KeyType& key) const;
	void expand_table();
	void contract_table(); //  { /* @@ yet to be implemented */ }
	void init();

public:
	LinearHashTable();
	LinearHashTable(double minloadf, double maxloadf);

	~LinearHashTable();
	void release();

	int num_items() const {
		return ((LinearHashTable*)this)->get_current_items();
	}
	int get_current_items() {
		lock.acquire_read_lock();
		int currentItems = keycount;
		lock.release_read_lock();
		return currentItems;
	}
	int num_buckets() const {
		return currentsize;
	}

	ValueType* find(const KeyType& key);
	inline const ValueType* find(const KeyType& key) const {
		return (const ValueType*)((LinearHashTable *)this)->find(key);
	}

	bool insert(const DataType& elem); // standard insert.
	bool insert(const KeyType& key, const ValueType& value) {
		return insert(DataType(key, value) );
	}

	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	//bool update(const KeyType& key, const ValueType& val) {
	//	return update(DataType(key, val) );
	//}

	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	//bool update(const DataType& rec) {
	//	return true;
	//}

	bool del(const KeyType& key); // standard del

	void display(std::ostream& stream) const;

	template<typename Archive> void save(Archive & ar,
			const unsigned int version = 0) const {
		ar & keycount;
		int d = 0;
		for (int i=0; i<DIRECTORY_SIZE; i++) {
			for (int j =0; j<SEGMENT_SIZE; j++) {
				if (!directory[i])
					continue;
				if (!directory[i]->seg[j])
					continue;
				LHTElem<DataType>* elem = directory[i]->seg[j]; // first on chain
				while (elem != NULL) {
					++d;
					ar & elem->data;
					elem = elem->next;
				}
			}
		}
	}

	template<typename Archive> void load(Archive & ar,
			const unsigned int version = 0) {
		int n;
		ar & n;

		for (int i =0; i<n; i++) {
			DataType dat;
			ar & dat;
			insert(dat);
		}
	}

	void copyTo(AccessMethod<KeyType, ValueType, LockType>& am) const {

		int d = 0;
		for (int i=0; i<DIRECTORY_SIZE; i++) {
			for (int j =0; j<SEGMENT_SIZE; j++) {
				if (!directory[i])
					continue;
				if (!directory[i]->seg[j])
					continue;
				LHTElem<DataType>* elem = directory[i]->seg[j]; // first on chain
				while (elem != NULL) {
					++d;
					am.insert(elem->data);
					elem = elem->next;
				}
			}
		}

	}

};

template <typename KeyType, typename ValueType, typename LockType> std::ostream& operator<<(
		std::ostream& strm,
		const LinearHashTable<KeyType, ValueType, LockType>& src) {
	src.display(strm);
	return strm;
}

/**
 *  @brief Default constructor, Initialize member variables.
 */
template <typename KeyType, typename ValueType, typename LockType> LinearHashTable<
		KeyType, ValueType, LockType>::LinearHashTable() {
	init();
}

template <typename KeyType, typename ValueType, typename LockType> void LinearHashTable<
		KeyType, ValueType, LockType>::init() {
	p = 0;
	maxp = SEGMENT_SIZE;
	keycount = 0;
	currentsize = SEGMENT_SIZE;
	minloadfctr = MIN_LOAD_FACTOR;
	maxloadfctr = MAX_LOAD_FACTOR;
	directory[0] = new Segment<DataType>();

	for (int i = 1; i < DIRECTORY_SIZE; i++)
		directory[i] = NULL;
}

/**
 * @brief Releases the LinearHashTable object.
 */
template <typename KeyType, typename ValueType, typename LockType> void LinearHashTable<
		KeyType, ValueType, LockType>::release() {
	for (int i = 0; i < DIRECTORY_SIZE; i++) {
		delete directory[i];
		directory[i] = NULL;
	}
	init();
}

/**
 *  @brief The constructor.
 */
template <typename KeyType, typename ValueType, typename LockType> LinearHashTable<
		KeyType, ValueType, LockType>::LinearHashTable(double minloadf,
		double maxloadf) {
	init();

	minloadfctr = minloadf;
	maxloadfctr = maxloadf;
}

/**
 *  @brief The destructor.
 */
template <typename KeyType, typename ValueType, typename LockType> LinearHashTable<
		KeyType, ValueType, LockType>::~LinearHashTable() {
	for (int i = 0; i < DIRECTORY_SIZE; i++) {
		delete directory[i];
		directory[i] = NULL;
	}
}

/**
 *  @brief Returns a hash value of a key.
 */
template <typename KeyType, typename ValueType, typename LockType> int LinearHashTable<
		KeyType, ValueType, LockType>::hash(const KeyType& key) const {
	int h, address;

	h = izenelib::util::HashFunction<KeyType>::convert_key(key) % HashFunction<KeyType>::PRIME;
	address = h % maxp;
	if (address < p)
		address = h % (2*maxp);

	return address;
}

/**
 * @brief Expands the table.
 */
template <typename KeyType, typename ValueType, typename LockType> void LinearHashTable<
		KeyType, ValueType, LockType>::expand_table() {
	int newaddress, oldsegmentindex, newsegmentindex;
	Segment<DataType>* oldsegment, *newsegment;
	LHTElem<DataType>* current, *previous; // for scanning down the old chain
	LHTElem<DataType>* lastofnew; // points to the last DataType of the new chain

	// reached maximum size of address space? if so, just continue the chaining.
	if (maxp + p < DIRECTORY_SIZE * SEGMENT_SIZE) {
		// locate the bucket to be split
		oldsegment = directory[p/SEGMENT_SIZE];
		oldsegmentindex = p % SEGMENT_SIZE;

		// Expand address space, if necessary create a new Segment<DataType>
		newaddress = maxp + p;
		newsegmentindex = newaddress % SEGMENT_SIZE;
		if (newsegmentindex == 0)
			directory[newaddress / SEGMENT_SIZE] = new Segment<DataType>();
		newsegment = directory[newaddress / SEGMENT_SIZE];

		// adjust the state variables
		p = p + 1;
		if (p == maxp) {
			maxp = 2 * maxp;
			p = 0;
		}

		currentsize = currentsize + 1;

		// relocate records to the new bucket
		current = oldsegment->seg[oldsegmentindex];
		previous = NULL;
		lastofnew = NULL;
		newsegment->seg[newsegmentindex] = NULL;

		while (current != NULL) {
			if (hash(current->data.get_key()) == newaddress) {
				// attach it to the end of the new chain
				if (lastofnew == NULL)
					newsegment->seg[newsegmentindex] = current;
				else
					lastofnew->next = current;
				if (previous == NULL)
					oldsegment->seg[oldsegmentindex] = current->next;
				else
					previous->next = current->next;
				lastofnew = current;
				current = current->next;
				lastofnew->next = NULL;
			} else {
				// leave it on the old chain
				previous = current;
				current = current->next;
			}
		}
	}
}

/**
 * @brief Contracts the table, exactly the opposite of expand_table().
 */
template <typename KeyType, typename ValueType, typename LockType> void LinearHashTable<
		KeyType, ValueType, LockType>::contract_table() {
	int oldsegmentindex, newsegmentindex;
	Segment<DataType> *oldsegment, *newsegment;
	LHTElem<DataType> *current, *previous; // for scanning down the new/current chain

	// Is the table contractable or has more than one segment
	if (currentsize > SEGMENT_SIZE) { // there is a bucket to shrink.
		// locate the bucket to free
		currentsize--;

		oldsegment = directory[currentsize/SEGMENT_SIZE];
		oldsegmentindex = currentsize % SEGMENT_SIZE;

		// adjust the state variables
		p = p - 1;
		if (p < 0) {
			maxp = maxp/2;
			p = maxp - 1;
		}

		newsegment = directory[p/SEGMENT_SIZE];
		newsegmentindex = p % SEGMENT_SIZE;

		// relocate records to the new bucket
		current = newsegment->seg[newsegmentindex];

		// sacn down the end of the current where the additonal records
		// will be attached to.
		previous = current;
		while (current != NULL) {
			previous = current;
			current = current->next;
		}
		// attach the chain of records to the end of the new bucket
		if (previous) {
			previous->next = oldsegment->seg[oldsegmentindex];
		} else
			newsegment->seg[newsegmentindex] = oldsegment->seg[oldsegmentindex];

		oldsegment->seg[oldsegmentindex] = NULL;

		// if necessary delete the old Segment<DataType>
		if (oldsegmentindex == 0) {
			delete directory[currentsize/SEGMENT_SIZE];
			directory[currentsize/SEGMENT_SIZE] = NULL;
		}
	}
}

template <typename KeyType, typename ValueType, typename LockType> ValueType* LinearHashTable<
		KeyType, ValueType, LockType>::find(const KeyType& key) {
	// first requests the read lock
	lock.acquire_read_lock();
	int address = hash(key);
	LHTElem<DataType>* elem =
			directory[address/SEGMENT_SIZE]->seg[address % SEGMENT_SIZE]; // first on chain
	while (elem != NULL) {
		if (key == elem->data.get_key()) {// found!
			// release the read lock
			lock.release_read_lock();
			return (ValueType*) &elem->data.get_value();
		} else
			elem = elem->next;
	}
	// release the read lock
	lock.release_read_lock();
	// not found
	return NULL;
}

/**
 * Inserts an DataType (takes a pointer).
 */
template <typename KeyType, typename ValueType, typename LockType> bool LinearHashTable<
		KeyType, ValueType, LockType>::insert(const DataType& elem) {
	// first requests the write lock
	lock.acquire_write_lock();
	if (maxloadfctr * currentsize <= keycount)
		expand_table(); // due for expanding table

	const KeyType& key = elem.get_key();
	int address = hash(key);
	Segment<DataType>* currentSegment = directory[address/SEGMENT_SIZE];
	int segIndex = address % SEGMENT_SIZE;
	LHTElem<DataType>* e = currentSegment->seg[segIndex]; // first on chain
	if (e == NULL)
		currentSegment->seg[segIndex] = new LHTElem<DataType>(elem);
	else {
		while (e->next != NULL) {
			if (e->data.get_key() == key) {// duplicate data
				// release the write lock
				lock.release_write_lock();
				return false;
			}
			e = e->next; // go to the end of the chain
		}
		if (e->data.get_key() == key) {
			// release the write lock
			lock.release_write_lock();
			return false;
		}
		e->next = new LHTElem<DataType>(elem);
	}

	keycount++; // increment key count by one.
	// release the write lock
	lock.release_write_lock();
	return true;
}

/**
 * @brief Deletes an DataType.
 * @code
 *	Comments: This method takes the ownership and deletes the data itself
 *            when successful.
 * @endcode
 */
template <typename KeyType, typename ValueType, typename LockType> bool LinearHashTable<
		KeyType, ValueType, LockType>::del(const KeyType& key) {
	// first requests the write lock
	lock.acquire_write_lock();
	if (minloadfctr * currentsize > keycount)
		contract_table();

	int address = hash(key);
	LHTElem<DataType>* elem =
			directory[address/SEGMENT_SIZE]->seg[address % SEGMENT_SIZE]; // first on chain
	LHTElem<DataType>* prev = elem;

	while (elem != NULL) {
		if (key == elem->data.get_key()) { // found!
			if (prev == elem) //
				directory[address/SEGMENT_SIZE]->seg[address % SEGMENT_SIZE]
						= prev->next;
			else
				prev->next = elem->next;

			delete elem;
			keycount--;
			// release the write lock
			lock.release_write_lock();
			return true;
		} else {
			prev = elem;
			elem = elem->next;
		}
	}
	// release the write lock
	lock.release_write_lock();
	// not found
	return false;
}

/**
 * @brief Display information about linear hashing for debug.
 *
 */
template <typename KeyType, typename ValueType, typename LockType> void LinearHashTable<
		KeyType, ValueType, LockType>::display(std::ostream& stream) const {
	stream << "Member variables: " << std::endl;
	stream << "the number of buckets: " << currentsize << std::endl;
	stream << "the number of records: " << keycount << std::endl;
	stream << "max load factor: " << maxloadfctr << std::endl;
	stream << "min load factor: " << minloadfctr << std::endl;
	stream << "max p value: " << maxp << std::endl;
	stream << "current p value: " << p << std::endl;
}

NS_IZENELIB_AM_END

#endif
