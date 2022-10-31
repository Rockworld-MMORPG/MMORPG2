fetchcontent_declare(
  sqlitecpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.2.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
fetchcontent_makeavailable(sqlitecpp)

set(SQLITECPP_RUN_CPPLINT OFF)
set(SQLITECPP_RUN_CPPCHECK OFF)
