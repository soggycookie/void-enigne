cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(VOID_PROJECT_NAME "void-engine")
set(VOID_ENGINE_FOLDER "void-engine")
set(VOID_ENGINE_SRC "void-engine/src/void")
set(VOID_ENGINE_PLATFORM_SRC "void-engine/src/platform")

project(${VOID_PROJECT_NAME})

set(VOID_PRECOMPILED_HEADER "${PROJECT_SOURCE_DIR}/${VOID_ENGINE_SRC}/pch.h")
set(VOID_ENGINE_INCLUDE "${PROJECT_SOURCE_DIR}/void-engine/src")

file(GLOB_RECURSE VOID_ENGINE_SRC_FILES CONFIGURE_DEPENDS
    "${VOID_ENGINE_SRC}/*.cpp"  
)

add_executable(${VOID_PROJECT_NAME} ${VOID_ENGINE_SRC_FILES})

set_target_properties(${VOID_PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>"
    VS_GLOBAL_IntDir         "${PROJECT_SOURCE_DIR}/bin/intermediate/$<CONFIG>"
    PDB_OUTPUT_DIRECTORY     "${PROJECT_SOURCE_DIR}/bin/intermediate/$<CONFIG>"
)

target_precompile_headers(${VOID_PROJECT_NAME} PRIVATE ${VOID_PRECOMPILED_HEADER})

target_include_directories(${VOID_PROJECT_NAME} 
  PRIVATE ${VOID_ENGINE_INCLUDE}
)

if(WIN32)
	file(GLOB_RECURSE VOID_ENGINE_PLATFORM_SRC_FILES CONFIGURE_DEPENDS
		"${VOID_ENGINE_PLATFORM_SRC}/d3d11/*.cpp"
		"${VOID_ENGINE_PLATFORM_SRC}/win32/*.cpp"
	)
    
	target_sources(${VOID_PROJECT_NAME}
        PRIVATE
            ${VOID_ENGINE_PLATFORM_SRC_FILES}
    )

    target_link_libraries(${VOID_PROJECT_NAME}
        PRIVATE
            d3d11
            dxgi
            dxguid
            user32
            gdi32
			d3dcompiler
    )

endif()

add_custom_command(TARGET ${VOID_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/void-engine/asset"
        "$<TARGET_FILE_DIR:${VOID_PROJECT_NAME}>/asset"
)

target_compile_definitions(void-engine
    PRIVATE
        $<$<CONFIG:Debug>:VOID_DEBUG>
)

target_compile_definitions(void-engine
   PRIVATE
       VOID_DEV_MODE
)
