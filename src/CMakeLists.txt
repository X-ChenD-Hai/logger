project(logger)

set(target1 interface)
pybind11_add_module(${target1} ${CMAKE_CURRENT_SOURCE_DIR}/server/platform/PyQt/interface.cpp)

target_link_libraries(${target1} PRIVATE ipc)
