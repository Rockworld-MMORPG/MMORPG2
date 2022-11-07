set(CMAKE_CXX_STANDARD 20)

set(ENABLE_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_UNINSTALL
    OFF
    CACHE BOOL "" FORCE)

fetchcontent_declare(
  mongo-c-driver
  URL https://github.com/mongodb/mongo-c-driver/releases/download/1.23.1/mongo-c-driver-1.23.1.tar.gz
  URL_HASH MD5=30980b22efed6ddef5d16eb174de62b1
  OVERRIDE_FIND_PACKAGE)

fetchcontent_makeavailable(mongo-c-driver)

fetchcontent_declare(
  mongo-cxx-driver
  URL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.7.0/mongo-cxx-driver-r3.7.0.tar.gz
  URL_HASH MD5=bce0008486358328a63481129ede6b0a
  OVERRIDE_FIND_PACKAGE)

fetchcontent_makeavailable(mongo-cxx-driver)

if(EXISTS ${mongo-cxx-driver_BINARY_DIR}/src/bsoncxx/bsoncxx_targets.cmake)
  set(bsoncxx_DIR ${mongo-cxx-driver_BINARY_DIR}/src/bsoncxx)
  find_package(bsoncxx CONFIG)
endif()

if(EXISTS ${mongo-cxx-driver_BINARY_DIR}/src/mongocxx/mongocxx_targets.cmake)
  set(mongocxx_DIR ${mongo-cxx-driver_BINARY_DIR}/src/mongocxx)
  find_package(mongocxx CONFIG)
endif()

add_library(mongocxx_interface INTERFACE)
add_library(Mongo::MongoCXX ALIAS mongocxx_interface)
target_link_libraries(mongocxx_interface INTERFACE mongo::mongocxx_shared)
