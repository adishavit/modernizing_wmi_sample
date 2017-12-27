project(WMISample)
cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)
 
add_executable(WMISample wmi_sample.cpp)
add_definitions(/D_CRT_SECURE_NO_WARNINGS)
add_definitions(/D_SCL_SECURE_NO_WARNINGS)
add_definitions(/DNOMINMAX)
add_definitions("/std:c++latest")
