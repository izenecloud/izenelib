MACRO_CHECK_SHOULD_BUILD(bigint)

IF(bigint_SHOULD_BUILD)

  SET(bigint_SRC
    BigIntegerAlgorithms.cpp
    BigUnsigned.cpp
    BigInteger.cpp
    BigUnsignedInABase.cpp
    BigIntegerUtils.cpp
    )

  MACRO_ADD_LIBRARY(bigint SRCS ${bigint_SRC})

ENDIF(bigint_SHOULD_BUILD)
