project(logger)

set(Python3_ROOT_DIR "D:/worktools/develop/anaconda3/envs/Qt")
set(Python3_EXECUTABLE "D:/worktools/develop/anaconda3/envs/Qt/python.exe")
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
add_executable(python_test ${CMAKE_CURRENT_SOURCE_DIR}/server/platform/PyQt/interface.cpp)
target_include_directories(python_test PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(python_test PRIVATE ${Python3_LIBRARIES})
target_link_libraries(python_test PRIVATE ipc)
