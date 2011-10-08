/**-
 * @file	IDFactory.h
 * @brief	Header file of ID Factory Class
 * @author	Quang & Do Hyun Yun
 * @date    2008-11-18
 * @details
 * ==============
 *
 * Using SDB/hash
 * @author Peisheng Wang
 * @date 2009-04-16
 *
 * ==============
 *
 * Refactor
 * @author Wei Cao
 * @date 2009-08-07
 *
 * ==============
 */

#ifndef _ID_FACTORY_H_
#define _ID_FACTORY_H_

#include <types.h>

#include "IDManagerTypes.h"
#include "IDFactoryException.h"
#include "IDFactoryErrorString.h"

NS_IZENELIB_IR_BEGIN

namespace idmanager {

/**
 * IDFactory will call its components, String2ID and ID2String instances for task.
 */
template <typename  NameString,
          typename  NameID,
          typename  IDGenerator,
          typename  IDStorage >
class IDFactory
{

public:

	/**
	 * @brief Constructor.
	 *
	 * @param storageName       name for storage.
	 */
	IDFactory(const string& storageName);

	virtual ~IDFactory();

	/**
	 * @brief a member function to get term ID from vocabulary which matches to the given term string.
	 *
	 * @param termStringBuffer	the content of term string.
	 * @param termStringLength	the length of term string.
	 * @param termId	    a term identifier which matches to the term string.
	 * @return true  : 	Term exists in the dictionary.
	 * @return false : 	Term does not exist in the dictionary.
	 */
	bool getTermIdByTermString(const typename NameString::value_type* termStringBuffer, const size_t termStringLength, NameID& termId)
	{
        bool ret = idGenerator_.conv(termStringBuffer, termStringLength, termId);
        idStorage_.put(termId, termStringBuffer, termStringLength );
        return ret;
	}

	/**
	 * @brief This function returns an ID given a string and stores <ID,string> pair into sdb.
	 * ID may not unique
	 * @param nameString the name string
	 * @param nameID the unique NameID
	 * @param insert whether insert docName to IDManager;
	 * @return true if the name id is successfully returned
	 * @return false if no more name id is available
	 */
	inline bool getNameIDByNameString(const NameString& nameString, NameID& nameID, bool insert = true);

	/**
	 * @brief This function returns an ID given a string and stores <ID,string> pair into sdb.
	 * set the  ID to the new value so that it can satisfy the incremental ID semantic.
	 * @param nameString the name string
	 * @param oldId the old NameID
	 * @param updatedId the new NameID
	 * @return true if the name id is successfully returned
	 * @return false if no more name id is available
	 */
	inline bool updateNameIDByNameString(const NameString& nameString, NameID& oldId, NameID& updatedId);

	/**
	 * @brief This function returns a name string given name ID
	 * @param nameID the unique Name ID
	 * @param nameString the name string
	 * @return true if the name string is successfully returned
	 * @return false if name id is not available
	 */
	inline bool getNameStringByNameID(const NameID& nameID, NameString& nameString);

	void flush()
	{
	    idGenerator_.flush();
	    idStorage_.flush();
	}

	void close()
	{
	    idGenerator_.close();
	    idStorage_.close();
	}

	void display()
	{
		idGenerator_.display();
		idStorage_.display();
	}

protected:

    string storageName_;

	IDGenerator idGenerator_; /// convert ID to String

	IDStorage idStorage_; /// convert String to ID

}; // end - template IDFactory

template <typename NameString, typename NameID,
    typename IDGenerator, typename IDStorage>
IDFactory<NameString, NameID, IDGenerator, IDStorage>::IDFactory(
    const string& storageName)
:
    storageName_(storageName),
    idGenerator_(storageName),
    idStorage_(storageName)
{
} // end - IDFactory()

template <typename NameString, typename NameID,
    typename IDGenerator, typename IDStorage>
IDFactory<NameString, NameID, IDGenerator, IDStorage>::~IDFactory()
{
} // end - ~IDFactory()

template <typename NameString, typename NameID,
    typename IDGenerator, typename IDStorage>
inline bool IDFactory<NameString, NameID, IDGenerator, IDStorage>::getNameIDByNameString(
        const NameString& nameString,
        NameID& nameID,
        bool insert)
{
	if( idGenerator_.conv(nameString, nameID, insert) )
           return true;

    if(insert)
        idStorage_.put(nameID, nameString);
    return false;
} // end - getNameIDByNameString()

template <typename NameString, typename NameID,
    typename IDGenerator, typename IDStorage>
inline bool IDFactory<NameString, NameID, IDGenerator, IDStorage>::updateNameIDByNameString(
        const NameString& nameString,
        NameID& oldId,
        NameID& updatedId)
{
    bool ret = idGenerator_.conv(nameString, oldId, updatedId);
    idStorage_.put(updatedId, nameString);
    return ret;
} // end - updateNameIDByNameString()


template <typename NameString, typename NameID,
    typename IDGenerator, typename IDStorage>
inline bool IDFactory<NameString, NameID, IDGenerator, IDStorage>::getNameStringByNameID(
        const NameID& nameID, NameString& nameString)
{
	return idStorage_.get(nameID,nameString);
} // end - getNameStringByNameID()

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _ID_FACTORY_H_
