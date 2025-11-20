cmake_minimum_required(VERSION 3.22)
project(ProtobufDemo LANGUAGES CXX)

# ============================================================
# 1. 基础设置 + 强制 libc++
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 自动检测 Clang 或环境变量强制启用 libc++
if(USE_LIBCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "启用 libc++（Clang 标准库）")
    add_compile_options("-stdlib=libc++")
    add_link_options("-stdlib=libc++" "-lc++abi")
endif()

# ============================================================
# 2. 查找我们自己编译的纯净生态
# ============================================================
list(PREPEND CMAKE_PREFIX_PATH
    "$ENV{LIBCXX_PKGS}"
    "/opt/libcxx-pkgs"
)

find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ============================================================
# 3. Proto 文件生成（2025 年唯一推荐写法）
# ============================================================
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
file(GLOB_RECURSE PROTO_FILES "${PROTO_DIR}/*.proto")

if(NOT PROTO_FILES)
    message(FATAL_ERROR "在 ${PROTO_DIR} 目录未找到 .proto 文件！")
endif()

set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/gen/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRCS "")
set(PROTO_HDRS "")

foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)
    get_filename_component(proto_path ${proto} PATH)

    # 支持多级目录
    file(RELATIVE_PATH proto_rel_path ${PROTO_DIR} ${proto_path})
    set(output_dir "${PROTO_GEN_DIR}")
    if(proto_rel_path)
        set(output_dir "${PROTO_GEN_DIR}/${proto_rel_path}")
        file(MAKE_DIRECTORY ${output_dir})
    endif()

    set(out_cc "${output_dir}/${proto_name}.pb.cc")
    set(out_h  "${output_dir}/${proto_name}.pb.h")

    add_custom_command(
        OUTPUT ${out_cc} ${out_h}
        COMMAND protobuf::protoc
            --cpp_out=${PROTO_GEN_DIR}
            --proto_path=${PROTO_DIR}
            ${proto}
        DEPENDS ${proto} protobuf::protoc
        COMMENT "Generating ${proto_name}.pb.cc/h"
        VERBATIM
    )

    list(APPEND PROTO_SRCS ${out_cc})
    list(APPEND PROTO_HDRS ${out_h})
endforeach()

# ============================================================
# 4. 可执行文件
# ============================================================
add_executable(demo
    src/main.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(demo PRIVATE ${PROTO_GEN_DIR})

target_link_libraries(demo PRIVATE
    protobuf::libprotobuf
    absl::strings
    absl::log
    absl::flags_parse
)

# ============================================================
# 5. 调试信息
# ============================================================
message(STATUS "编译器: ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS "Protobuf 版本: ${Protobuf_VERSION}")
message(STATUS "生成的 Proto 文件数量: ${PROTO_FILES}")
