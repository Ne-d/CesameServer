cmake_minimum_required(VERSION 3.18)

project(CesameServer LANGUAGES CUDA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CesameServer main.cpp
    CPUMonitor.h CPUMonitor.cpp
    GPUMonitor.h GPUMonitor.cpp
    MemoryMonitor.h MemoryMonitor.cpp
    common.h
    Network.h Network.cpp)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include(GNUInstallDirs)

target_include_directories(CesameServer PUBLIC /opt/cuda/include/)
target_link_directories(CesameServer PRIVATE ${CUDA_TOOLKIT_ROOT_DIR}/lib64/stubs/)
target_link_libraries(CesameServer libnvidia-ml.so)

install(TARGETS CesameServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
