cmake_minimum_required(VERSION 3.24)
project(BluePlayerTests LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Core Quick Gui Multimedia REQUIRED)

# FFmpeg est trouvé par le CMakeLists.txt principal, nous utilisons ses variables directement.
# Assurez-vous que FFmpeg_LIBRARIES et FFmpeg_INCLUDE_DIRS sont définis par le parent.

# Exemple de test (à remplacer par de vrais tests)
add_executable(mytest test_example.cpp)
target_link_libraries(mytest PRIVATE
  Qt6::Core
  ${FFmpeg_LIBRARIES}
)
target_link_directories(mytest PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
target_include_directories(mytest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${FFmpeg_INCLUDE_DIRS}
)

include(CTest)
add_test(NAME mytest COMMAND mytest)
