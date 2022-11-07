set(CMAKE_CXX_STANDARD 20)

set(ENABLE_TESTS OFF)
set(ENABLE_EXAMPLES OFF)
set(ENABLE_UNINSTALL OFF)

fetchcontent_declare(
  mongo-c-driver
  URL https://github.com/mongodb/mongo-c-driver/releases/download/1.23.1/mongo-c-driver-1.23.1.tar.gz
  URL_HASH MD5=30980b22efed6ddef5d16eb174de62b1)

fetchcontent_declare(
  mongo-cxx-driver
  URL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.7.0/mongo-cxx-driver-r3.7.0.tar.gz
  URL_HASH MD5=bce0008486358328a63481129ede6b0a)

fetchcontent_makeavailable(mongo-c-driver)
fetchcontent_makeavailable(mongo-cxx-driver)

set(bsoncxx_DIR ${mongo-cxx-driver_BINARY_DIR}/src/bsoncxx)
set(mongocxx_DIR ${mongo-cxx-driver_BINARY_DIR}/src/mongocxx)

find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)

add_library(mongocxx_interface INTERFACE)
add_library(Mongo::MongoCXX ALIAS mongocxx_interface)

target_link_libraries(mongocxx_interface INTERFACE mongo::mongocxx_shared)
