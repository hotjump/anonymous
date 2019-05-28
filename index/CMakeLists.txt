# CMake最低版本号要求
cmake_minimum_required (VERSION 2.6)

# 项目信息
project (anonymous)

#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(CMAKE_CXX_FLAGS "-std=c++11 -g -ggdb -Wno-deprecated-declarations ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/hash
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/lz4/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../index
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_file
)

# 生成链接库
add_library (index
    ${CMAKE_CURRENT_SOURCE_DIR}/../index/IndexBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../index/IndexBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../index/IndexReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/hash/Hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/lz4/lib/lz4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/lz4/lib/lz4hc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/lz4/lib/lz4frame.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/entropy_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/error_private.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/fse_decompress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/pool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/threading.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/xxhash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/common/zstd_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/fse_compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstd_compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstd_fast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstd_ldm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstd_opt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/huf_compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstd_double_fast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstd_lazy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/zstdmt_compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/compress/hist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/decompress/huf_decompress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/decompress/zstd_decompress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/decompress/zstd_ddict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/zstd/lib/decompress/zstd_decompress_block.c
    )