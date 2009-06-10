# - tells wether need to expires the cache if version unmatch
# MACRO_FIND_PACKAGE_CHECK_CACHE_VERSION
#

MACRO(MACRO_FIND_PACKAGE_CHECK_CACHE_VERSION _out_match _name)
  IF (ARGV2)
    SET(_prefix ${ARGV2})
  ELSE (ARGV2)
    STRING(TOUPPER ${_name} _prefix)
  ENDIF (ARGV2)

  SET(${_out_match} TRUE)

  IF (${_name}_FIND_VERSION) # only check when use sepcify a version in find_package

    # if version cannot be found in cache, sure it's not match
    IF (${_prefix}_VERSION)
      INCLUDE(MacroVersionCmp)
      MACRO_VERSION_CMP(${${_prefix}_VERSION} ${${_name}_FIND_VERSION} _cmp_result)
      IF (_cmp_result LESS 0)
        SET(${_out_match} FALSE)
      ELSEIF (${_name}_FIND_VERSION_EXACT AND _cmp_result GREATER 0)
        SET(${_out_match} FASE)
      ENDIF (_cmp_result LESS 0)
    ELSE (${_prefix}_VERSION)
      SET(${_out_match} FALSE)
    ENDIF (${_prefix}_VERSION)

  ENDIF (${_name}_FIND_VERSION)
ENDMACRO(MACRO_FIND_PACKAGE_CHECK_CACHE_VERSION)