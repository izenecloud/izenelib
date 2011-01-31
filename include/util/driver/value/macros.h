#ifndef IZENELIB_DRIVER_VALUE_MACROS_H
#define IZENELIB_DRIVER_VALUE_MACROS_H
/**
 * @file izenelib/driver/value/macros.h
 * @author Ian Yang
 * @date Created <2010-06-08 15:09:18>
 * @brief PP macros help to define value types
 */
#include <boost/preprocessor.hpp>

// ==================================================
// Helpers

#define IZENELIB_DRIVER_ENUM(list, func) \
    BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(list), func, list)

#define IZENELIB_DRIVER_REPEAT(list, func) \
    BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(list), func, list)


// ==================================================
// Generates type list such as
// int,unsigned int,bool

// Gets type, the first element in pair.
#define IZENELIB_DRIVER_GET_VALUE_TYPE(z, i, list) \
    BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_SEQ_ELEM(i, list))

#define IZENELIB_DRIVER_VALUE_TYPE_LIST(list) \
    IZENELIB_DRIVER_ENUM(list, IZENELIB_DRIVER_GET_VALUE_TYPE)


// ==================================================
// Generates name string list such as
// "Int","Uint","Bool"

// Gets type name, the second element in pair.
#define IZENELIB_DRIVER_GET_VALUE_NAME(z, i, list) \
    BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_ELEM(i, list))

// Gets type name and stringize it.
#define IZENELIB_DRIVER_GET_VALUE_NAME_STR(z, i, list) \
    BOOST_PP_STRINGIZE(IZENELIB_DRIVER_GET_VALUE_NAME(z, i, list))

#define IZENELIB_DRIVER_VALUE_NAME_STR_LIST(list) \
    IZENELIB_DRIVER_ENUM(list, IZENELIB_DRIVER_GET_VALUE_NAME_STR)

// ==================================================
// Generates enum for types such as
// kIntType = 0, kUintType = 1, kBoolType = 2

// Get the enum name from type name, e.g.
// Type name Null corresponds to "kNullType = i", where i is
// the position number.
#define IZENELIB_DRIVER_VALUE_TYPE_ENUM(z, i, list) \
    BOOST_PP_CAT(BOOST_PP_CAT(k, IZENELIB_DRIVER_GET_VALUE_NAME(z, i, list)), \
                 Type) = i

#define IZENELIB_DRIVER_VALUE_TYPE_ENUM_LIST(list) \
    IZENELIB_DRIVER_ENUM(list, IZENELIB_DRIVER_VALUE_TYPE_ENUM)

// ==================================================
// Generates type alias such as
// typedef int IntType; typedef unsigned UintType; typedef bool BoolType;

// Generates type alias. e.g., for pair (int, Int), following alias is generated
// typedef int IntType;
#define IZENELIB_DRIVER_VALUE_TYPE_DEF(z, i, list) \
    typedef BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_SEQ_ELEM(i, list)) \
        BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_ELEM(i, list)), Type);

#define IZENELIB_DRIVER_VALUE_TYPE_DEF_LIST(list) \
    IZENELIB_DRIVER_REPEAT(list, IZENELIB_DRIVER_VALUE_TYPE_DEF)


// ==================================================
// Generates overloaded functions to forward "as"-family function.

#define IZENELIB_DRIVER_VALUE_AS_FUNC(t) \
    inline Value::BOOST_PP_CAT(t, Type) \
    as(const Value& value, const Value::BOOST_PP_CAT(t, Type)&) \
    { return BOOST_PP_CAT(as, t)(value); }

// wrapper
#define IZENELIB_DRIVER_VALUE_AS_FUNC_W(z, i, list) \
    IZENELIB_DRIVER_VALUE_AS_FUNC(BOOST_PP_SEQ_ELEM(i, list))

#define IZENELIB_DRIVER_VALUE_AS_FUNCTIONS(list) \
    IZENELIB_DRIVER_REPEAT(list, IZENELIB_DRIVER_VALUE_AS_FUNC_W)

// ==================================================
// Generates named asOr functions
#define IZENELIB_DRIVER_VALUE_AS_OR_FUNC(t) \
    inline Value::BOOST_PP_CAT(t, Type) \
    BOOST_PP_CAT(BOOST_PP_CAT(as, t), Or)( \
        const Value& value, \
        const Value::BOOST_PP_CAT(t, Type)& def \
    ) \
    { \
        return nullValue(value) ? def : BOOST_PP_CAT(as, t)(value); \
    }

// wrapper
#define IZENELIB_DRIVER_VALUE_AS_OR_FUNC_W(z, i, list) \
    IZENELIB_DRIVER_VALUE_AS_OR_FUNC(BOOST_PP_SEQ_ELEM(i, list))

#define IZENELIB_DRIVER_VALUE_AS_OR_FUNCTIONS(list) \
    IZENELIB_DRIVER_REPEAT(list, IZENELIB_DRIVER_VALUE_AS_OR_FUNC_W)

// ==================================================
// Generates named get functions

#define IZENELIB_DRIVER_VALUE_GET_CONST_REF_FOR_TYPE(t) \
    inline const BOOST_PP_CAT(t, Type)& BOOST_PP_CAT(get, t)() const \
    { return get<BOOST_PP_CAT(t, Type)>(); }
#define IZENELIB_DRIVER_VALUE_GET_MUTABLE_REF_FOR_TYPE(t) \
    inline BOOST_PP_CAT(t, Type)& BOOST_PP_CAT(get, t)() \
    { return get<BOOST_PP_CAT(t, Type)>(); }
#define IZENELIB_DRIVER_VALUE_GET_OR_RESET_REF_FOR_TYPE(t) \
    inline BOOST_PP_CAT(t, Type)& BOOST_PP_CAT(BOOST_PP_CAT(get, t), OrReset)() \
    { return getOrReset<BOOST_PP_CAT(t, Type)>(); }
#define IZENELIB_DRIVER_VALUE_GET_CONST_PTR_FOR_TYPE(t) \
    inline const BOOST_PP_CAT(t, Type)* BOOST_PP_CAT(BOOST_PP_CAT(get, t), Ptr)() const \
    { return getPtr<BOOST_PP_CAT(t, Type)>(); }
#define IZENELIB_DRIVER_VALUE_GET_MUTABLE_PTR_FOR_TYPE(t) \
    inline BOOST_PP_CAT(t, Type)* BOOST_PP_CAT(BOOST_PP_CAT(get, t), Ptr)() \
    { return getPtr<BOOST_PP_CAT(t, Type)>(); }
#define IZENELIB_DRIVER_VALUE_GET_OR_RESET_PTR_FOR_TYPE(t) \
    inline BOOST_PP_CAT(t, Type)* BOOST_PP_CAT(BOOST_PP_CAT(get, t), PtrOrReset)() \
    { return getPtrOrReset<BOOST_PP_CAT(t, Type)>(); }

#define IZENELIB_DRIVER_VALUE_GET_FUNC(t) \
    IZENELIB_DRIVER_VALUE_GET_CONST_REF_FOR_TYPE(t) \
    IZENELIB_DRIVER_VALUE_GET_MUTABLE_REF_FOR_TYPE(t) \
    IZENELIB_DRIVER_VALUE_GET_OR_RESET_REF_FOR_TYPE(t) \
    IZENELIB_DRIVER_VALUE_GET_CONST_PTR_FOR_TYPE(t) \
    IZENELIB_DRIVER_VALUE_GET_MUTABLE_PTR_FOR_TYPE(t) \
    IZENELIB_DRIVER_VALUE_GET_OR_RESET_PTR_FOR_TYPE(t) \

// wrapper
#define IZENELIB_DRIVER_VALUE_GET_FUNC_W(z, i, l) \
    IZENELIB_DRIVER_VALUE_GET_FUNC(IZENELIB_DRIVER_GET_VALUE_NAME(z,i,l))

#define IZENELIB_DRIVER_VALUE_GET_FUNCTIONS(list) \
    IZENELIB_DRIVER_REPEAT(list, IZENELIB_DRIVER_VALUE_GET_FUNC_W)

#endif // IZENELIB_DRIVER_VALUE_MACROS_H
