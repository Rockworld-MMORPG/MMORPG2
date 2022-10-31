# Force disable cpplint and cppcheck because they cause issues when compiling with MSVC
set(SQLITECPP_RUN_CPPLINT
    OFF
    CACHE BOOL "Run cpplint.py tool for Google C++ StyleGuide." FORCE)
set(SQLITECPP_RUN_CPPCHECK
    OFF
    CACHE BOOL "Run cppcheck C++ static analysis tool." FORCE)

fetchcontent_declare(
  sqlitecpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.2.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)

fetchcontent_makeavailable(sqlitecpp)
