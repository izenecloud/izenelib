#include <am/sdb_btree/sdb_btree.h>

using namespace std;
using namespace izenelib::am;


/**
 * Key type used in MCache, SDB Btree and Cache DB
 */
class KeyType
{
public:
	/**
	 * Site Identifier
	 */
	unsigned int siteId;

	/**
	 * URL in string
	 */
	std::string url;

	/**
	 * Default constructor
	 */
	KeyType() : siteId(0) {}
	
	/**
	 * Constructor
     *
     * \param[in] _siteId The site identifier
     * \param[in] _url The url in string
     */
	KeyType(unsigned int _siteId, std::string _url) : siteId(_siteId), url(_url) {}

	/**
	 * Copy constructor
	 *
	 * \param[in] key Another instance of class KeyType
	 */
	KeyType (const KeyType & key) : siteId(key.siteId), url(key.url) {}

	/**
	 * Method for serialization
	 *
	 * \param[in] ar An instance of class Archive
	 * \param[in] version Version of the instance of the Archive
	 */
	template<class Archive> 
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & siteId;
		ar & url;
	}

    /**
	 * Method for comparasion with another instance of KeyType
	 * 
	 * \param[in] other Another instance of KeyType
	 *
	 * \return The result of comparasion
	 */
	int compare(const KeyType& other) const
	{
		if (siteId < other.siteId)
			return -1;
		else if (siteId > other.siteId)
			return 1;
		else
		{	
			if (url < other.url)
				return -1;
			else if (url > other.url)
				return 1;
			else
				return 0;
		}
	}

	bool operator < (const KeyType & other) const
	{
		return compare (other) < 0;
	}

	bool operator == (const KeyType & other) const
	{
		return compare (other) == 0;
	}

	void display() const 
	{
		printf ("[%d][%s]\n", (int)siteId, url.c_str());
	}
};


MAKE_MEMCPY_SERIALIZATION(KeyType)

NS_IZENELIB_UTIL_BEGIN

template<> 
inline void 
read_image_memcpy<KeyType>(KeyType & dat, const char * str, const size_t size) 
{
	char * p = (char*)str;
	memcpy(&dat.siteId, p, sizeof(unsigned int));
	p += sizeof(unsigned int);			
	dat.url = string(p, size-sizeof(unsigned int)-1);
}

template<> 
inline void 
write_image_memcpy<KeyType>(const KeyType & dat, char * &str, size_t & size)
{
	size = sizeof(unsigned int) + dat.url.size() + 1;
	str = new char[size];
	memset(str, 0, size);
	
	char *p = str;	
	memcpy(p, &dat.siteId, sizeof(unsigned int));
	p += sizeof(unsigned int);
	memcpy(p, dat.url.c_str(), dat.url.size() + 1);

	KeyType data1;
	read_image_memcpy (data1, str, size);
	assert (data1 == dat);
}

NS_IZENELIB_UTIL_END

int main(){

sdb_btree<KeyType, string> sdb("abc.dat");
sdb.open();
KeyType key;
key.siteId = 3;
key.url = "http://www.fntoday.co.kr/news/articleView.html?idxno=6111";

string value = "[NONE]SENSITIVE";
bool rt = sdb.insert(key, value);
sdb.commit();
string* str;
str = sdb.find(key);
cout<<rt<<endl;
cout<<*str<<endl;




}
