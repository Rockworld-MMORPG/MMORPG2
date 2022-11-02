fetchcontent_declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.88
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE)

fetchcontent_makeavailable(imgui)

set(IMGUI_DIR
    ${imgui_SOURCE_DIR}
    PARENT_SCOPE)
