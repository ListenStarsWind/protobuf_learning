cmake_minimum_required(VERSION 3.22)
project(CompareJSONvsPB LANGUAGES CXX)

# ==============================================================================
# 1. 使用 Clang-21 + libc++
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "需要使用 Clang 21：export CXX=clang++-21")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options("-stdlib=libc++")
add_link_options("-stdlib=libc++" "-lc++abi")

# ==============================================================================
# 2. 引入 protobuf（来自 /opt/libcxx-pkgs）
# ==============================================================================
list(PREPEND CMAKE_PREFIX_PATH "/opt/libcxx-pkgs")
find_package(Protobuf CONFIG REQUIRED)

# ==============================================================================
# 3. protoc 生成源码（最小版本）
# ==============================================================================
set(PROTO_DIR  "${CMAKE_SOURCE_DIR}/proto")
set(GEN_DIR    "${CMAKE_BINARY_DIR}/gen")

file(MAKE_DIRECTORY "${GEN_DIR}/proto")
file(GLOB proto_files "${PROTO_DIR}/*.proto")

foreach(proto ${proto_files})
    get_filename_component(name_we ${proto} NAME_WE)
    set(out_cc "${GEN_DIR}/proto/${name_we}.pb.cc")
    set(out_h  "${GEN_DIR}/proto/${name_we}.pb.h")

    add_custom_command(
        OUTPUT ${out_cc} ${out_h}
        COMMAND protobuf::protoc
                --cpp_out=${GEN_DIR}/proto
                --proto_path=${PROTO_DIR}
                ${proto}
        DEPENDS ${proto}
        VERBATIM
    )

    list(APPEND PB_SRCS ${out_cc})
    list(APPEND PB_HDRS ${out_h})
endforeach()

# ==============================================================================
# 4. 可执行文件（只 main.cc + PB）
# ==============================================================================
add_executable(compare
    src/main.cc
    ${PB_SRCS}
    ${PB_HDRS}
)

target_include_directories(compare PRIVATE
    ${GEN_DIR}     # 让 pb 头文件可见
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(compare PRIVATE
    protobuf::libprotobuf
)

# ==============================================================================
# 5. 编译信息
# ==============================================================================
message(STATUS "Clang + libc++ 已启用")
message(STATUS "Protobuf 头文件目录: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Proto 输入目录: ${PROTO_DIR}")
message(STATUS "生成目录: ${GEN_DIR}/proto")

