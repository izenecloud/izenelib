#ifndef IZENELIB_DRIVER_VALUE_VALUE_H
#define IZENELIB_DRIVER_VALUE_VALUE_H
/**
 * @file izenelib/driver/value/Value.h
 * @author Ian Yang
 * @date Created <2010-06-08 15:08:21>
 * @brief Value wrapping the variant types
 */
#include "ValueTypeHelper.h"
#include "macros.h"
#include "def.h"

#include <boost/assert.hpp>
#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_unsigned.hpp>

namespace izenelib {
namespace driver {

using boost::bad_get;

class Value
{
public:
    enum
    {
        /// @brief Total count of types.
        kTypeCount = ValueTypeHelper::kTypeCount
    };

    /// @brief Type enum, expanded such as kNullType = 0
    enum Type { IZENELIB_DRIVER_VALUE_TYPE_ENUM_LIST(IZENELIB_DRIVER_VALUE_TYPES) };

    /// @brief Type alias, such as typedef xxx NullType;
    IZENELIB_DRIVER_VALUE_TYPE_DEF_LIST(IZENELIB_DRIVER_VALUE_TYPES)

    /// @brief Underlying variant.
    typedef boost::variant<
        IZENELIB_DRIVER_VALUE_TYPE_LIST(IZENELIB_DRIVER_VALUE_TYPES)
    > variant_type;

    typedef variant_type::types type_list;

    /// @{
    /// @brief type helper functions

    static inline const std::string& type2Name(Type type)
    {
        return ValueTypeHelper::instance().type2Name(type);
    }
    static inline Type name2Type(const std::string& typeName)
    {
        return static_cast<Type>(ValueTypeHelper::instance().name2Type(typeName));
    }
    static bool validType(Type type)
    {
        return ValueTypeHelper::instance().validType(type);
    }

    /// @}

public:
    /// @brief Constructs Null
    Value();

    /// @brief Explicitly constructs Null using keyword \c nullValue.
    Value(NullValueFunc);

    /// @brief Constructs a copy of another value.
    Value(const Value& other);

    /// @brief Constructs String.
    Value(const char* str);

    /// @brief Constructs Bool.
    Value(bool boolValue);

    /// @brief Constructs non-integral value.
    template<typename T>
    Value(const T& operand,
          typename boost::disable_if<boost::is_integral<T> >::type* = 0)
    : value_(operand)
    {}

    /// @brief Constructs Int.
    template<typename T>
    Value(T operand,
          typename boost::enable_if<boost::is_signed<T> >::type* = 0)
    : value_(IntType(operand))
    {}

    /// @brief Constructs Uint.
    template<typename T>
    Value(T operand,
          typename boost::enable_if<boost::is_unsigned<T> >::type* = 0)
    : value_(UintType(operand))
    {}

    template<typename T>
    Value& operator=(const T& operand)
    {
        // forward to assign.
        assign(operand);
        return *this;
    }

    /// @brief Sets this value a copy of another.
    void assign(const Value& other)
    {
        value_ = other.value_;
    }

    /// @brief Sets this value to Null using keyword \c nullValue.
    void assign(NullValueFunc)
    {
        value_ = NullType();
    }

    /// @brief Sets to String.
    void assign(const char* str);

    /// @brief Sets to Bool.
    void assign(bool boolValue)
    {
        value_ = BoolType(boolValue);
    }

    /// @brief Assigns a non-integral value.
    template<typename T>
    void assign(const T& operand,
                typename boost::disable_if<boost::is_integral<T> >::type* = 0)
    {
        value_ = operand;
    }

    /// @brief Assigns Int.
    template<typename T>
    void assign(T operand,
                typename boost::enable_if<boost::is_signed<T> >::type* = 0)
    {
        value_ = IntType(operand);
    }

    /// @brief Assigns Uint.
    template<typename T>
    void assign(T operand,
                typename boost::enable_if<boost::is_unsigned<T> >::type* = 0)
    {
        value_ = UintType(operand);
    }

    /// @brief Returns enum representing the type of this value.
    Type type() const
    {
        return static_cast<Type>(value_.which());
    }

    void swap(Value& other)
    {
        value_.swap(other.value_);
    }

    /// @brief Resets to Null.
    void reset()
    {
        value_ = def<NullType>();
    }

    /// @brief Resets to the default value of given type.
    template<typename T>
    inline void reset()
    {
        value_ = def<T>();
    }

    /// @brief Gets reference to the underlying object of the specified type.
    /// @return Returns reference to the value
    /// @exception boost::bad_get Thrown when given type is incorrect.
    template<typename T>
    inline const T& get() const;

    /// @brief See const version.
    template<typename T>
    inline T& get();

    /// @brief Gets this value as specified type. If the type is not correct,
    /// reset this value to that type first.
    template<typename T>
    inline T& getOrReset();

    /// @brief Gets pointer to the underlying object of the specified type.
    /// @return Returns pointer to value if given type is correct. Returns 0
    ///         otherwise.
    template<typename T>
    inline const T* getPtr() const;

    /// @brief See const version.
    template<typename T>
    inline T* getPtr();

    /// @brief Gets this value as specified type. If the type is not correct,
    /// reset this value to that type first.
    template<typename T>
    inline T* getPtrOrReset();

    // Get by name, e.g.
    // getInt, getIntOrReset, getIntPtr, getIntPtrOrReset.
    IZENELIB_DRIVER_VALUE_GET_FUNCTIONS(IZENELIB_DRIVER_VALUE_TYPES)

    /// @brief Gets element by key.
    ///
    /// - This value is set to an empty object if it is not.
    /// - If the key does not exist, Null is inserted and returned.
    Value& operator[](const std::string& key)
    {
        return getOrReset<ObjectType>()[key];
    }

    /// @brief Gets element by key.
    /// @return Reference to the value with the specified key. Returns reference
    ///         to a Null if this value is not an object, or the key does not
    ///         exist
    const Value& operator[](const std::string& key) const;

    /// @brief Tests whether this value is an object, the key exists and does
    /// not point to Null.
    bool hasKey(const std::string key) const;

    /// @brief Gets element by index.
    ///
    /// - This value is set to an empty array if it is not.
    /// - If the index is out of range, nulls are appended at the end of the
    ///   array.
    Value& operator()(std::size_t index);

    /// @brief Gets element by index.
    /// @return Reference to the value with the specified index. Returns
    ///         reference to a Null if this value is not an array, or the index
    ///         is out of range.
    const Value& operator()(std::size_t index) const;

    /// @brief Appends a element at the end of the array and returns reference
    /// to it.
    ///
    /// This value is converted If this value is not an array, it is converted
    /// to array first.
    Value& operator()();

    /// @brief Gets size of \a value
    /// @return Returns number of elements in Array and Object. Returns 0 for
    ///         Null. Returns 1 for other.
    inline std::size_t size() const;

    inline void clear();

public:
    /// @brief Gets underlying variant.
    variant_type& variant()
    {
        return value_;
    }

    /// @brief Gets underlying variant.
    const variant_type& variant() const
    {
        return value_;
    }

private:
    variant_type value_;
};

inline void swap(Value& a, Value& b)
{
    a.swap(b);
}

}} // namespace izenelib::driver

#define IZENELIB_DRIVER_VALUE_INLINE
#include "get.inl"
#include "size.inl"
#include "clear.inl"
#undef IZENELIB_DRIVER_VALUE_INLINE

#include "NullValue.h"
#include "compare.h"

#endif // IZENELIB_DRIVER_VALUE_VALUE_H
