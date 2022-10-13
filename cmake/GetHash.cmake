function(get_hash RESULT_VARIABLE_NAME)
  find_package(Git REQUIRED)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    RESULT_VARIABLE result
    OUTPUT_VARIABLE COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  string(LENGTH COMMIT_HASH RESULT_LENGTH)

  if(${RESULT_LENGTH} GREATER 0)
    string(PREPEND COMMIT_HASH 0x)
  endif()

  set(${RESULT_VARIABLE_NAME}
      ${COMMIT_HASH}
      PARENT_SCOPE)
endfunction()
