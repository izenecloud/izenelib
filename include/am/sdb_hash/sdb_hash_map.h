/**
 * @file  sdb_hash.h
 * @brief The header file of definition of linear hashing table based on memory.
 */

#ifndef sdb_hash_h
#define sdb_hash_h 1

#include "sh_types.h"

NS_IZENELIB_AM_BEGIN

/*
 * @brief shmap_elem is a wrapper typename for DataType and provide next elem
 * for List like behaviour within the sdb_hash.
 */
template <typename KeyType, typename ValueType> class shmap_elem {

	typedef DataType<KeyType,ValueType> DataType;
	CompareFunctor<KeyType> comp;
public:
	DataType data;
	shmap_elem<KeyType,ValueType>* left;
	shmap_elem<KeyType,ValueType>* right;

public:
	shmap_elem(const shmap_elem& source) :
	data(source.data), left(source.left), right(source.right) {
	}
	shmap_elem(const DataType& d) : data(d), left(NULL),right(NULL) {
	}

	shmap_elem& operator=(const shmap_elem<KeyType, ValueType>& source) {
		if (this != &source) {
			data= source.data;
			left = source.left;
			right = source.right;
		}
		return *this;
	}

	bool insert(const DataType& elem)
	{
		shmap_elem<KeyType, ValueType>* se = this;
		while( true ) {
			if( comp(elem.get_key(), se->data.get_key())> 0 ) {
				if(se->right)se = se->right;
				else
				{	se->right = new shmap_elem(elem);
					return true;
				}
			}
			else if( comp(elem.get_key(), se->data.get_key()) < 0 )
			{
				if(se->left) se = se->left;
				else {
					se->left = new shmap_elem(elem);
					return true;
				}
			}
			else {
				return false;
			}
		}
		return false;
	}

	bool del(const KeyType& key)
	{
		shmap_elem<KeyType, ValueType> *se = this;
		shmap_elem<KeyType, ValueType> *p, *s;
		while( true ) {
			if( comp( key, se->data.get_key() )> 0 ) {
				se = se->right;
			}
			else if( comp( key, se->data.get_key() ) < 0 )
			{
				se = se->left;
			}
			else {
				if( !se->left )
				{	p = se;
					se = se->right;
					delete p;
				}
				else if( !se->right )
				{
					p = se;
					se = se->left;
					delete p;
				}
				else
				{
					p = se; s = se->left;
					while( s->right ) {p = s; s = s->right;}
					se->data = s->data;
					if(p != se) p->right = s->left;
					else
					p->left = s->left;
					delete s;
				}
			}
		}
		return false;
	}

	ValueType* search(const KeyType& key)
	{
		shmap_elem<KeyType, ValueType>* se = this;
		while( se ) {
			if( comp(key, se->data.get_key())> 0 ) {
				se = se->right;
			}
			else if( comp(key, se->data.get_key() ) < 0 )
			{
				se = se->left;
			}
			else {
				return new ValueType( se->data.get_value() );
			}
		}
		return NULL;
	}

	void display(ostream& os = cout)
	{
		if( left ) {
			left->display(os);
			os<<"----|";
		}
		os<<data.key;
		os<<endl;
		if( right ) {
			right->display(os);
			os<<"----|";
		}
	}

	~shmap_elem() {
	} // next items are deleted in the sh_map typename.
};

template <typename KeyType, typename ValueType, typename LockType =NullLock> class sdb_hash :
	public AccessMethod<KeyType, ValueType, LockType> {
	typedef DataType<KeyType,ValueType> DataType;
	shmap_elem<KeyType, ValueType>* directory[SH_DIRECTORY_SIZE];
	int keycount;
	LockType lock; // for thread-safe
protected:
	int hash(const KeyType& key) const;
	void init();

public:
	sdb_hash();
	~sdb_hash();
	void release();

	int num_items() const {
		return keycount;
	}
	//ValueType* find(const KeyType& key);
	//inline const ValueType* find(const KeyType& key) const {
	//	return (const ValueType*)((sdb_hash *)this)->find(key);
	//}

	ValueType* find(const KeyType& key);

	bool insert(const DataType& elem);
	bool insert(const KeyType& key, const ValueType& value) {
		return insert(DataType(key, value) ) ;
	}
	bool del(const KeyType& key); // standard del

	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	bool update(const KeyType& key, const ValueType& val) {
		return update(DataType(key, val) );
	}
	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	bool update(const DataType& rec) {
		return false;
	}

	void display(std::ostream& stream = cout) const;

	void flush() {
	}

};

template <typename KeyType, typename ValueType, typename LockType> std::ostream& operator<<(
		std::ostream& strm, const sdb_hash<KeyType, ValueType, LockType>& src) {
	src.display(strm);
	return strm;
}

/**
 *  @brief Default constructor, Initialize member variables.
 */
template <typename KeyType, typename ValueType, typename LockType> sdb_hash<
		KeyType, ValueType, LockType>::sdb_hash() {
	init();
}

template <typename KeyType, typename ValueType, typename LockType> void sdb_hash<
		KeyType, ValueType, LockType>::init() {
	keycount = 0;
	for (int i = 1; i < SH_DIRECTORY_SIZE; i++)
		directory[i] = NULL;
}

/**
 * @brief Releases the sdb_hash object.
 */
template <typename KeyType, typename ValueType, typename LockType> void sdb_hash<
		KeyType, ValueType, LockType>::release() {
	for (int i = 0; i < SH_DIRECTORY_SIZE; i++) {
		delete directory[i];
		directory[i] = NULL;
	}
	init();
}

/**
 *  @brief The destructor.
 */
template <typename KeyType, typename ValueType, typename LockType> sdb_hash<
		KeyType, ValueType, LockType>::~sdb_hash() {
	for (int i = 0; i < SH_DIRECTORY_SIZE; i++) {
		delete directory[i];
		directory[i] = NULL;
	}
}

/**
 *  @brief Returns a hash value of a key.
 */
template <typename KeyType, typename ValueType, typename LockType> int sdb_hash<
		KeyType, ValueType, LockType>::hash(const KeyType& key) const {
	int h, address;

	uint32_t convkey = 0;
	const char* str = (const char*)key.c_str();
	for (size_t i = 0; i < key.size(); i++)
		convkey = 37*convkey + *str++;
	address = convkey % SH_DIRECTORY_SIZE;
	return address;
}

template <typename KeyType, typename ValueType, typename LockType> ValueType* sdb_hash<
		KeyType, ValueType, LockType>::find(const KeyType& key) {
	lock.acquire_read_lock();
	int address = hash(key);
	ValueType* rv= directory[address]->search(key);
	lock.release_read_lock();
	return rv;
}

/**
 * Inserts an ValueType (takes a pointer).
 */
template <typename KeyType, typename ValueType, typename LockType> bool sdb_hash<
		KeyType, ValueType, LockType>::insert(const DataType& elem) {
	lock.acquire_write_lock();
	int address = hash(elem.get_key() );

	if ( !directory[address]) {
		directory[address] = new shmap_elem<KeyType, ValueType>(elem);
	}
	if (directory[address]->insert(elem) ) {
		keycount++;
		lock.release_write_lock();
		return true;
	}
	lock.release_write_lock();
	return false;
}

/**
 * @brief Deletes an ValueType.
 * @code
 *	Comments: This method takes the ownership and deletes the data itself
 *            when successful.
 * @endcode
 */
template <typename KeyType, typename ValueType, typename LockType> bool sdb_hash<
		KeyType, ValueType, LockType>::del(const KeyType& key) {
	lock.acquire_write_lock();
	int address = hash(key);
	if (directory[address]->del(key) ) {
		keycount--;
		lock.release_write_lock();
		return true;
	}
	lock.release_write_lock();
	return false;
}

/**
 * @brief Display information about linear hashing for debug.
 *
 */
template <typename KeyType, typename ValueType, typename LockType> void sdb_hash<
		KeyType, ValueType, LockType>::display(std::ostream& stream) const {
	stream << "Member variables: " << std::endl;
	stream << "the number of buckets: " << SH_DIRECTORY_SIZE << std::endl;
	stream << "the number of records: " << keycount << std::endl;
	for (int i=0; i<SH_DIRECTORY_SIZE; i++) {
		cout<<"\n\n";
		if (directory[i])
			directory[i]->display(stream);
	}
}

NS_IZENELIB_AM_END

#endif
