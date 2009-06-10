# - Compare versions
# MACRO_VERSION_CMP(version1 version2 result)
#
# result < 0 if version1 < version2
# result == 0 if version1 == version2
# result > 0 if version1 > version2
#

MACRO(MACRO_VERSION_CMP _version1 _version2 _result)
  IF (NOT _version1)
    SET(_version1 0)
  ENDIF (NOT _version1)

  IF (NOT _version2)
    SET(_version2 0)
  ENDIF (NOT _version2)

  STRING(REGEX MATCHALL "[0-9]+" _version1_parts "${_version1}")
  STRING(REGEX MATCHALL "[0-9]+" _version2_parts "${_version2}")

  LIST(LENGTH _version1_parts _version1_count)
  LIST(LENGTH _version2_parts _version2_count)

  IF (_version1_count LESS ${_version2_count})
    SET(_version_count ${_version1_count})
    SET(${_result} -1)
  ELSEIF (_version1_count GREATER ${_version2_count})
    SET(_version_count ${_version2_count})
    SET(${_result} 1)    
  ELSE (_version1_count LESS ${_version2_count})
    SET(_version_count ${_version1_count})
    SET(${_result} 0)
  ENDIF (_version1_count LESS ${_version2_count})
  MATH(EXPR _version_count "${_version_count} - 1")

  IF (NOT _version_count LESS 0)
    FOREACH(i RANGE 0 ${_version_count})
      LIST(GET _version1_parts ${i} _v1)
      LIST(GET _version2_parts ${i} _v2)
      IF (NOT _v1)
        SET(_v1 0)
      ENDIF (NOT _v1)

      IF (NOT _v2)
        SET(_v2 0)
      ENDIF (NOT _v2)

      IF (_v1 LESS ${_v2})
        SET(${_result} -1)
        BREAK()
      ELSEIF (_v1 GREATER ${_v2})
        SET(${_result} 1)
        BREAK()
      ENDIF (_v1 LESS ${_v2})
    ENDFOREACH(i)
  ENDIF (NOT _version_count LESS 0)

ENDMACRO(MACRO_VERSION_CMP)