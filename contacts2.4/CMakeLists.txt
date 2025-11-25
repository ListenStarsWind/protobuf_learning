# 最低 cmake 版本要求
cmake_minimum_required(VERSION 3.22)
project(ContactsDemo LANGUAGES CXX)

# ==============================================================================
# 1. 强制用 Clang-21 + libc++（避免跑偏）
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "本项目必须用 Clang 21 + libc++ 编译，先执行：export CXX=clang++-21")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options("-stdlib=libc++")
add_link_options("-stdlib=libc++" "-lc++abi")
message(STATUS "Clang 21 + libc++ 已启用")

# ==============================================================================
# 2. 查找我们自己装的 Protobuf + Abseil
# ==============================================================================
list(PREPEND CMAKE_PREFIX_PATH "/opt/libcxx-pkgs")
find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ==============================================================================
# 3. 自己控制 protoc 生成流程（稳定，可追踪依赖）
# ==============================================================================
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/gen/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# 找所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES RELATIVE "${PROTO_DIR}" "${PROTO_DIR}/*.proto")
if (NOT PROTO_FILES)
    message(FATAL_ERROR "在 ${PROTO_DIR} 里没找到 .proto 文件")
endif()

# 存放生成出来的 .cc/.h
set(PROTO_SRCS "")
set(PROTO_HDRS "")

# 逐个调用 protoc
foreach(proto_rel IN LISTS PROTO_FILES)
    get_filename_component(proto_dir  "${proto_rel}" DIRECTORY)      # 可能的子目录
    get_filename_component(proto_name "${proto_rel}" NAME_WE)        # 不带后缀名

    set(out_dir   "${PROTO_GEN_DIR}/${proto_dir}")
    set(out_cc    "${out_dir}/${proto_name}.pb.cc")
    set(out_h     "${out_dir}/${proto_name}.pb.h")
    set(dep_file "${out_cc}.d") # cmake protobuf 源文件依赖描述文件

    # 确保输出目录
    file(MAKE_DIRECTORY "${out_dir}")

    add_custom_command(
        OUTPUT  "${out_cc}" "${out_h}"
        COMMAND protobuf::protoc
                --cpp_out=${PROTO_GEN_DIR}
                --proto_path=${PROTO_DIR}
                --dependency_out=${dep_file}
                "${PROTO_DIR}/${proto_rel}"
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --blue --bold "Fixing depfile paths: ${dep_file}"
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/fix-proto-dep.cmake"
                "${dep_file}" "${CMAKE_SOURCE_DIR}"
        DEPENDS "${PROTO_DIR}/${proto_rel}" protobuf::protoc
        DEPFILE "${dep_file}"                          # 让 CMake 自动读取这个 .d 文件
        COMMENT "protoc → ${proto_rel}"
        VERBATIM
        USES_TERMINAL
    )

    list(APPEND PROTO_SRCS "${out_cc}")
    list(APPEND PROTO_HDRS "${out_h}") 
endforeach()

# ==============================================================================
# 4. 主程序入口
# ==============================================================================
add_executable(demo
    src/main.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PROTO_GEN_DIR}          # 生成的 .pb.h 都放这里
)

target_link_libraries(demo PRIVATE
    protobuf::libprotobuf
    absl::strings
    absl::log
)

# ==============================================================================
# 5. 打印一下关键信息
# ==============================================================================
message(STATUS "")
message(STATUS "==================================================")
message(STATUS " 项目          : ${PROJECT_NAME}")
message(STATUS " 编译器        : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS " 标准库        : libc++")
message(STATUS " Proto 文件    : ${PROTO_FILES}")
message(STATUS " 生成目录      : ${PROTO_GEN_DIR}")
message(STATUS " 可执行文件    : demo")
message(STATUS "==================================================")
message(STATUS "")
