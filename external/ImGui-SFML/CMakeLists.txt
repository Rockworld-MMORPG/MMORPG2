set(IMGUI_SFML_FIND_SFML OFF)

fetchcontent_declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)

fetchcontent_makeavailable(imgui-sfml)
