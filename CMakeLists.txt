cmake_minimum_required(VERSION 3.3.3)

project(Poop VERSION "0.1.0" DESCRIPTION "Demo" LANGUAGES "C")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

list(APPEND source_files
  src/main.c
  src/glad.c
  src/modplay.c
)

list(APPEND include_folders
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/inc/"
)

list(APPEND link_libraries
  OpenGL::OpenGL
  SDL2::SDL2
)

if(TARGET SDL2::SDL2main)
  list(APPEND link_libraries SDL2::SDL2main)
endif()

add_executable(Poop ${source_files})
target_include_directories(Poop PUBLIC ${include_folders})
target_link_libraries(Poop PUBLIC ${link_libraries})
