cmake_minimum_required(VERSION 3.16)
project(MultiWanProject CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# Fixed: Changed IMPORTED_TARGETS to IMPORTED_TARGET
pkg_check_modules(LIBNM REQUIRED IMPORTED_TARGET libnm)

# Fixed: Also fixed a small typo in WifiManager.cpp if needed
add_executable(multiwan WifiManager.cpp)

target_link_libraries(multiwan PRIVATE PkgConfig::LIBNM)
