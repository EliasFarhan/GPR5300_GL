cmake_minimum_required(VERSION 3.10)
project(GPR5300)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(BUILD_SHARED_LIBS FALSE CACHE INTERNAL "")
if(WIN32)
add_compile_definitions(NOMINMAX)
endif(WIN32)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/externals)

include_directories(include ${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC src/*.cpp include/*.h)
file(GLOB CMN_SHADERS shaders/*.vert shaders/*.frag)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
		$<$<CONFIG:RelWithDebInfo>:/MT>
    )
endif()

if(MSVC)
	if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
		set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
	else()
		set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
	endif()
elseif(UNIX)
	set(GLSL_VALIDATOR "glslangValidator")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
		"${PROJECT_SOURCE_DIR}/shaders/*.frag"
		"${PROJECT_SOURCE_DIR}/shaders/*.vert"
		)

foreach(GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
	get_filename_component(EXTENSION ${GLSL} EXT)
	file(RELATIVE_PATH PATH_NAME "${PROJECT_SOURCE_DIR}/shaders/" ${PATH_NAME})
	#MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
	set(GLSL_OUTPUT "${PROJECT_BINARY_DIR}/shaders/${PATH_NAME}/${FILE_NAME}")
	set(ENGINE_SHADER "${PROJECT_SOURCE_DIR}/shaders/engine/engine${EXTENSION}.glsl")
	add_custom_command(
			OUTPUT ${GLSL_OUTPUT}
			COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/${PATH_NAME}"
			COMMAND ${CMAKE_COMMAND} -D SRC1=${ENGINE_SHADER}
                             -D SRC2=${GLSL}
                             -D DST=${GLSL_OUTPUT}
                             -P ${PROJECT_SOURCE_DIR}/cmake/concat.cmake
			COMMAND ${GLSL_VALIDATOR} ${GLSL_OUTPUT}
			DEPENDS ${GLSL} "${PROJECT_SOURCE_DIR}/shaders/engine/engine${EXTENSION}.glsl")
	list(APPEND SPIRV_BINARY_FILES ${GLSL_OUTPUT})
endforeach(GLSL)

add_custom_target(
		Shaders
		DEPENDS ${SPIRV_BINARY_FILES}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s ASSERTIONS=1 -g4 -O0 ")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --emrun -s EXPORT_ALL=1 --use-preload-plugins --preload-file shaders/ ")

  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
  set(USE_SDL2 ON CACHE BOOL  "Use SDL2")
  set(USE_EMSCRIPTEN TRUE CACHE INTERNAL "")

else()
  set(USE_EMSCRIPTEN false)
endif()

if(NOT USE_EMSCRIPTEN)
	#sdl2
	set(SDL_VERSION 2.0.9)
	set(SDL_DIR ${EXTERNAL_DIR}/SDL2-${SDL_VERSION}/)
	set(SDL_STATIC ON)
	set(SDL_SHARED OFF)
	add_subdirectory(${SDL_DIR})
	include_directories(${SDL2_DIR}/include ${SDL2_DIR}/src)
	LIST(APPEND SFGE_LIBRARIES
			SDL2-static)
	if(MSVC)
		LIST(APPEND SFGE_LIBRARIES
			SDL2main)
	endif(MSVC)
	#opengl
	set(OpenGL_GL_PREFERENCE GLVND)
	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	LIST(APPEND SFGE_LIBRARIES
			${OPENGL_LIBRARIES})
		message("OpenGL Libraries: ${OPENGL_LIBRARIES}")

	#stb_image
	set(SFML_DIR ${EXTERNAL_DIR}/SFML)
	include_directories(${SFML_DIR}/extlibs/headers)

	set(REMOTERY_DIR ${EXTERNAL_DIR}/Remotery)
	add_subdirectory(${REMOTERY_DIR})
	include_directories(${REMOTERY_DIR})

	LIST(APPEND SFGE_LIBRARIES Remotery)
endif()
#glad
set(GLAD_DIR ${EXTERNAL_DIR}/glad)
include_directories(${GLAD_DIR}/include)
add_subdirectory(${GLAD_DIR})
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD=1)
LIST(APPEND SFGE_LIBRARIES
		glad)

#assimp
set(ASSIMP_BUILD_ASSIMP_TOOLS FALSE CACHE INTERNAL "")
set(ASSIMP_BUILD_SAMPLES FALSE CACHE INTERNAL "")
set(ASSIMP_BUILD_TESTS FALSE CACHE INTERNAL "")
set(ASSIMP_NO_EXPORT TRUE CACHE INTERNAL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE CACHE INTERNAL "")
set(ASSIMP_BUILD_FBX_IMPORTER TRUE CACHE INTERNAL "")
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE CACHE INTERNAL "")
set(ASSIMP_BUILD_STL_IMPORTER TRUE CACHE INTERNAL "")
set(ASSIMP_BUILD_BLEND_IMPORTER TRUE CACHE INTERNAL "")
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE CACHE INTERNAL "")
set(ASSIMP_DIR ${EXTERNAL_DIR}/assimp)
include_directories(${ASSIMP_DIR}/include ${CMAKE_BINARY_DIR}/externals/assimp/include)
add_subdirectory(${ASSIMP_DIR})

LIST(APPEND SFGE_LIBRARIES
			assimp)
#gli
set(GLI_DIR ${EXTERNAL_DIR}/gli)
IF(NOT EXISTS ${GLI_DIR}/CMakeLists.txt)
message(FATAL_ERROR "Please do git submodule update --init --recursive")
ENDIF()
include_directories(${GLI_DIR})
#glm
set(GLM_DIR ${EXTERNAL_DIR}/glm)
IF(NOT EXISTS ${GLM_DIR}/CMakeLists.txt)
message(FATAL_ERROR "Please do git submodule update --init --recursive")
ENDIF()
include_directories(${GLM_DIR})

#Imgui
set(IMGUI_ROOT ${EXTERNAL_DIR}/imgui/)
message("IMGUI ROOT: ${IMGUI_ROOT}")
add_compile_definitions("USE_GL_ES3")
set(SDL2_IMGUI ${EXTERNAL_DIR}/imgui-sdl/)
add_subdirectory(${SDL2_IMGUI})
include_directories(${IMGUI_ROOT} ${SDL2_IMGUI})
LIST(APPEND SFGE_LIBRARIES
	imgui-sdl)



set(UTILITIES_DIR ${EXTERNAL_DIR}/utilities/)
include_directories(${UTILITIES_DIR})

#MISCELLANEOUS UTILITIES
if(NOT USE_EMSCRIPTEN)

endif()

#COMMON
message("SFGE LIBRARIES : ${SFGE_LIBRARIES}")
file(GLOB_RECURSE SCENES_SRC
		"${PROJECT_SOURCE_DIR}/scenes/*.json"
		)
foreach(SCENE_SRC ${SCENES_SRC})
	get_filename_component(FILE_NAME ${SCENE_SRC} NAME)
	get_filename_component(PATH_NAME ${SCENE_SRC} DIRECTORY)
	
	#MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
	set(SCENE_OUTPUT "${PROJECT_BINARY_DIR}/scenes/${FILE_NAME}")
	add_custom_command(
			OUTPUT ${SCENE_OUTPUT}
			COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/scenes/"
			COMMAND ${CMAKE_COMMAND} -E copy ${SCENE_SRC} ${SCENE_OUTPUT}
			DEPENDS ${SCENE_SRC})
			list(APPEND SCENES_OUTPUT ${SCENE_OUTPUT})
endforeach(SCENE_SRC)
add_custom_target(
		Scenes
		DEPENDS ${SCENES_OUTPUT}
)
file(GLOB_RECURSE SHADERS_SRC shaders/engine/*.vert* shaders/engine/*.frag*)
source_group("Shaders" FILES ${SHADERS_SRC})
source_group("Scenes" FILES ${SCENES_SRC})
add_library(COMMON ${SRC} ${SHADERS_SRC} ${SCENES_SRC})
target_link_libraries(COMMON PUBLIC ${SFGE_LIBRARIES})
MESSAGE("Libraries: ${SFGE_LIBRARIES}")
set_property(TARGET COMMON PROPERTY CXX_STANDARD 17)
add_dependencies(COMMON Shaders Scenes)
if(USE_SDL2)
target_compile_definitions(COMMON PUBLIC USE_SDL2=1)
endif()
if(USE_EMSCRIPTEN)
target_compile_definitions(COMMON PUBLIC USE_EMSCRIPTEN=1)
endif()
#copy folder to build
add_custom_command(TARGET COMMON POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data)
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data/)
#SFGE COURSES
SET(SFGE_COURSE_DIR ${CMAKE_SOURCE_DIR}/main)
file(GLOB COURSE_FILES ${SFGE_COURSE_DIR}/*.cpp )
foreach(course_file ${COURSE_FILES} )
    # I used a simple string replace, to cut off .cpp.
    file(RELATIVE_PATH course_relative_path ${SFGE_COURSE_DIR} ${course_file} )
    string( REPLACE ".cpp" "" course_name ${course_relative_path} )
	file(GLOB_RECURSE SHADERS_SRC shaders/${course_name}/*.vert shaders/${course_name}/*.frag)
	source_group("Shaders" FILES ${SHADERS_SRC})

    add_executable(${course_name} ${SFGE_COURSE_DIR}/${course_relative_path} ${SHADERS_SRC})
    # Make sure YourLib is linked to each app
    target_link_libraries(${course_name} PUBLIC COMMON)
	set_property(TARGET ${course_name} PROPERTY CXX_STANDARD 17)
	IF(UNIX)
		set_target_properties(${course_name} PROPERTIES COMPILE_FLAGS "-ffast-math")
		TARGET_LINK_OPTIONS(${course_name} INTERFACE "-s MAIN_MODULE=1")
	ENDIF(UNIX)
	set_target_properties (${course_name} PROPERTIES
    FOLDER Courses)

  add_custom_command(TARGET ${course_name} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data)
endforeach()
