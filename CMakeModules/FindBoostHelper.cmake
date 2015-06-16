FIND_PATH(Boost_INCLUDE_DIR
  NAMES
    filesystem.hpp
  PATHS
    /usr/include
    /usr/local/include
)

FIND_PACKAGE( Boost 1.52 REQUIRED COMPONENTS "system" "filesystem" )
