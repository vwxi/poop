cmake_minimum_required(VERSION 3.27)

project(Poop VERSION "0.1.0" DESCRIPTION "Demo" LANGUAGES "C")

find_package(OpenGL)

list(APPEND source_files
  src/main.c
)

list(APPEND include_folders
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/inc/"
)

list(APPEND link_libraries
  OpenGL::OpenGL
)

add_executable(Poop ${source_files})
target_include_directories(Poop PUBLIC ${include_folders})
target_link_libraries(Poop PUBLIC ${link_libraries})
