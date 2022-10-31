fetchcontent_declare(
  mysql-connector
  GIT_REPOSITORY https://github.com/mysql/mysql-connector-cpp.git
  GIT_TAG 8.0.31
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)

fetchcontent_makeavailable(mysql-connector)
