# libraries
SET(SNOPT_LIBS_LIST
  snopt7
  snprint7
)

SET(SNOPT_LIBRARIES )
FOREACH(LIB in ${SNOPT_LIBS_LIST})
  FIND_LIBRARY(SNOPT_LIB_${LIB}
    NAMES ${LIB}
    HINTS $ENV{SNOPT}/lib)
  IF(SNOPT_LIB_${LIB})
    MESSAGE(STATUS "Found ${LIB}: ${SNOPT_LIB_${LIB}}")
    SET(SNOPT_LIBRARIES ${SNOPT_LIBRARIES} ${SNOPT_LIB_${LIB}})
  ELSE(SNOPT_LIB_${LIB})
    MESSAGE(STATUS "Could not find lib${LIB}")
  ENDIF(SNOPT_LIB_${LIB})
ENDFOREACH(LIB)

IF (SNOPT_LIBRARIES)
   SET(SNOPT_LIBRARIES ${SNOPT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LAPACK_LIBRARIES} ${EXTRA_LIBRARIES} ${CMAKE_DL_LIBS})
   MESSAGE(STATUS "Found Snopt libs: ${SNOPT_LIBRARIES}")
   SET(SNOPT_FOUND_LIBS TRUE)
ELSE (SNOPT_LIBRARIES)
   MESSAGE(STATUS "Could not find Snopt libs")
ENDIF (SNOPT_LIBRARIES)


IF(SNOPT_FOUND_LIBS)
  SET(SNOPT_FOUND TRUE)
ENDIF(SNOPT_FOUND_LIBS)
