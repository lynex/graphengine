if (HAVE_PROTOBUF)
    return()
endif()

include(ExternalProject)
include(GNUInstallDirs)

if ((${CMAKE_INSTALL_PREFIX} STREQUAL /usr/local) OR
    (${CMAKE_INSTALL_PREFIX} STREQUAL "C:/Program Files (x86)/ascend"))
    set(CMAKE_INSTALL_PREFIX ${GE_SOURCE_DIR}/output CACHE STRING "path for install()" FORCE)
    message(STATUS "No install prefix selected, default to ${CMAKE_INSTALL_PREFIX}.")
endif()

set(protobuf_CXXFLAGS "-Wno-maybe-uninitialized -Wno-unused-parameter -fPIC -fstack-protector-all -D_FORTIFY_SOURCE=2 -D_GLIBCXX_USE_CXX11_ABI=0 -O2")
set(protobuf_LDFLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
ExternalProject_Add(protobuf_build
                    URL https://github.com/protocolbuffers/protobuf/archive/v3.8.0.tar.gz
                    #URL /home/txd/workspace/linux_cmake/pkg/protobuf-3.8.0.tar.gz
                    #SOURCE_DIR ${METADEF_DIR}/../../third_party/protobuf/src/protobuf-3.8.0
                    #DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory ${METADEF_DIR}/../../third_party/protobuf/src/protobuf-3.8.0 <SOURCE_DIR>
                    #CONFIGURE_COMMAND ${CMAKE_COMMAND}
                    #-DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
                    #-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                    #-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                    #-DCMAKE_LINKER=${CMAKE_LINKER}
                    #-DCMAKE_AR=${CMAKE_AR}
                    #-DCMAKE_RANLIB=${CMAKE_RANLIB}
                    #-Dprotobuf_WITH_ZLIB=OFF
                    #-Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_FLAGS=${protobuf_CXXFLAGS} -DCMAKE_CXX_LDFLAGS=${protobuf_LDFLAGS} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/protobuf <SOURCE_DIR>/cmake
                    CONFIGURE_COMMAND cd <SOURCE_DIR>
                            && ./autogen.sh && cd <BINARY_DIR> && <SOURCE_DIR>/configure --prefix=${CMAKE_INSTALL_PREFIX}/protobuf --with-zlib=no CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CXXFLAGS=${protobuf_CXXFLAGS} LDFLAGS=${protobuf_LDFLAGS}
                            && bash -c "sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=\"\"|g' libtool && sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool"
                    BUILD_COMMAND $(MAKE)
                    INSTALL_COMMAND $(MAKE) install
                    EXCLUDE_FROM_ALL TRUE
)
include(GNUInstallDirs)

set(PROTOBUF_SHARED_PKG_DIR ${CMAKE_INSTALL_PREFIX}/protobuf)

add_library(protobuf SHARED IMPORTED)

file(MAKE_DIRECTORY ${PROTOBUF_SHARED_PKG_DIR}/include)

set_target_properties(protobuf PROPERTIES
                      IMPORTED_LOCATION ${PROTOBUF_SHARED_PKG_DIR}/lib/libprotobuf.so
)

target_include_directories(protobuf INTERFACE ${PROTOBUF_SHARED_PKG_DIR}/include)

set(INSTALL_BASE_DIR "")
set(INSTALL_LIBRARY_DIR lib)

install(FILES ${PROTOBUF_SHARED_PKG_DIR}/lib/libprotobuf.so ${PROTOBUF_SHARED_PKG_DIR}/lib/libprotobuf.so.19.0.0 OPTIONAL
        DESTINATION ${INSTALL_LIBRARY_DIR})

add_dependencies(protobuf protobuf_build)

#set(HAVE_PROTOBUF TRUE CACHE BOOL "protobuf build add")
set(HAVE_PROTOBUF TRUE)
