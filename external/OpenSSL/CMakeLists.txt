# Commit 777190bc4bdda3d48de3b5dd9c804669942b2210 creates a target for 1.1.1s
fetchcontent_declare(
  openssl
  GIT_REPOSITORY https://github.com/viaduck/openssl-cmake.git
  GIT_TAG 777190bc4bdda3d48de3b5dd9c804669942b2210
  GIT_SHALLOW ON
  GIT_PROGRESS ON)

fetchcontent_makeavailable(openssl)
