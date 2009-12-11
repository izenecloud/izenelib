MACRO_CHECK_SHOULD_BUILD(bigint)

IF(bigint_SHOULD_BUILD)

  SET(bigint_SRC
    BigIntegerAlgorithms.cpp
    BigUnsigned.cpp
    BigInteger.cpp
    BigUnsignedInABase.cpp
    BigIntegerUtils.cpp
    )

  ADD_LIBRARY(bigint ${bigint_SRC})
  #INSTALL(TARGETS bigint DESTINATION lib)
  INSTALL(TARGETS bigint       
      LIBRARY DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(bigint_SHOULD_BUILD)
