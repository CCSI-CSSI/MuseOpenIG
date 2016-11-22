SET(Boost_USE_MULTITHREADED      ON)
IF(WIN32)
  set(Boost_USE_STATIC_LIBS        ON) # only find static libs
  set(Boost_USE_STATIC_RUNTIME     OFF)
ENDIF()
FIND_PACKAGE( Boost 1.52 REQUIRED COMPONENTS "system" "filesystem" "date_time" "regex" "thread" "chrono")
