#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <map>
#include <string>

namespace izenelib{namespace osgi{

/**
 * The <code>Properties</code> object describes a service
 * object. Each property entry in the <code>Properties</code>
 * object consists of a key/value pair.
 * When a service is registered with the framework
 * the service object is specified by the class name and a
 * <code>Properties</code> object. This allows to register
 * several service object of the same type (class name) but
 * with different properties.<br>
 * So the service listeners are able to distinguish between
 * service objects of the same type by using the provided
 * properties.
 *
 * @author magr74
 */
class Properties
{
private:

    /**
     * The internal storage of the properties.
     */
    std::map<std::string,std::string> mapper;

public:

    /**
     * Stores a property entry.
     *
     * @param key
     *     The identifier of the property.
     *
     * @param value
     *     The value of the property.
     */
    void put( const std::string &key, const std::string &value );

    /**
     * Retrieves (not removing) a property value.
     *
     * @param key
     *     The key of the property.
     *
     * @return
     *     The property value relating to the key.
     */
    std::string get( const std::string &key );

    /**
     * Returns a std::string representation of the properties
     * object.
     *
     * @return
     *     A std::string containing all key/value pairs in
     *     a readable form.
     */
    std::string toString() const;

    /**
     * Compares this property object with a
     * specified object.
     */
    bool operator==( const Properties& props );

    /**
     * Returns the number of entries in the
     * <code>Properties</code> object.
     */
    int getSize() const;

    /**
     * Returns an iterator for the internal std::map structure.
     *
     * @return
     * The iterator instance.
     */
    std::map<std::string,std::string>::const_iterator begin() const;

    /**
     * Returns an iterator for the internal std::map structure.
     *
     * @return
     * The iterator instance.
     */
    std::map<std::string,std::string>::const_iterator end() const;

};

}}
#endif
