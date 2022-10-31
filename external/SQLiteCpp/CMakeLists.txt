set(SQLITECPP_RUN_CPPLINT OFF)
set(SQLITECPP_RUN_CPPCHECK OFF)

fetchcontent_declare(
  sqlitecpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.2.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)

fetchcontent_makeavailable(sqlitecpp)
