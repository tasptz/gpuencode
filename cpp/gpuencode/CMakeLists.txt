set(src
    encode.cpp
    encode.h
    bitstreambuffer.cpp
    bitstreambuffer.h
    inputbuffer.cpp
    inputbuffer.h
    event.cpp
    event.h
    session.cpp
    session.h
    api.cpp
    api.h
    context.cpp
    context.h
    helper.h
    nocopymove.h
    logging_intern.h
)

if(NOT TARGET gpuencode_logging)
    list(APPEND src "logging/logging.h")
    set(private_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/logging")
endif()

list(APPEND private_libs ${cuda_libs})
if(TARGET gpuencode_logging)
    list(APPEND private_libs gpuencode_logging)
endif()

if(GPUENCODE_STATIC)
    set(linking "STATIC")
else()
    set(linking "SHARED")
endif()
add_library(gpuencode ${linking} ${src})
set_target_properties(gpuencode PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_link_libraries(gpuencode PRIVATE ${private_libs})
target_include_directories(gpuencode INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/.." PRIVATE ${private_include_dirs})
install(TARGETS gpuencode
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
)
install(FILES encode.h context.h DESTINATION "include/gpuencode")

if(BUILD_PYTHON)
    pybind11_add_module(pygpuencode MODULE ${src} wrapper.cpp)
    target_link_libraries(pygpuencode PRIVATE ${private_libs})
    target_include_directories(pygpuencode PRIVATE ${private_include_dirs})
    install(TARGETS pygpuencode
        RUNTIME DESTINATION "gpuencode"
        LIBRARY DESTINATION "gpuencode"
    )
    install(FILES "${CMAKE_SOURCE_DIR}/python/init.py"
        DESTINATION "gpuencode"
        RENAME "__init__.py"
    )
endif()