cmake_minimum_required(VERSION 3.10)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

# AngelScript options
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(AS_NO_EXCEPTIONS OFF CACHE BOOL "" FORCE)
set(AS_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

project(bspguy)

if (MSVC)
	# static runtime for all projects
	add_compile_options(/MT)
	add_definitions(-UHAVE___BUILTIN_ASSUME_ALIGNED) # probably only needed for v141_xp
endif()

set(BUILD_TESTING OFF)
set(XZ_TOOL_XZ OFF)
set(XZ_TOOL_XZDEC OFF)
set(XZ_TOOL_LZMADEC OFF)
set(XZ_TOOL_LZMAINFO OFF)
if (UNIX)
	set(XZ_SMALL ON) # fixes build error for 32bit
endif()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/xz)

# Add AngelScript library
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/angelscript/sdk/angelscript/projects/cmake angelscript)

# AngelScript include directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/angelscript/sdk/angelscript/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/angelscript/sdk/add_on)

if (UNIX)
	#set_target_properties(liblzma PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/xz/src/liblzma/api)

# AngelScript add-ons source files
set(ANGELSCRIPT_ADDON_FILES
	angelscript/sdk/add_on/contextmgr/contextmgr.cpp
	angelscript/sdk/add_on/contextmgr/contextmgr.h
	angelscript/sdk/add_on/datetime/datetime.cpp
	angelscript/sdk/add_on/datetime/datetime.h
	angelscript/sdk/add_on/debugger/debugger.cpp
	angelscript/sdk/add_on/debugger/debugger.h
	angelscript/sdk/add_on/scriptany/scriptany.cpp
	angelscript/sdk/add_on/scriptany/scriptany.h
	angelscript/sdk/add_on/scriptarray/scriptarray.cpp
	angelscript/sdk/add_on/scriptarray/scriptarray.h
	angelscript/sdk/add_on/scriptbuilder/scriptbuilder.cpp
	angelscript/sdk/add_on/scriptbuilder/scriptbuilder.h
	angelscript/sdk/add_on/scriptdictionary/scriptdictionary.cpp
	angelscript/sdk/add_on/scriptdictionary/scriptdictionary.h
	angelscript/sdk/add_on/scriptfile/scriptfile.cpp
	angelscript/sdk/add_on/scriptfile/scriptfile.h
	angelscript/sdk/add_on/scriptfile/scriptfilesystem.cpp
	angelscript/sdk/add_on/scriptfile/scriptfilesystem.h
	angelscript/sdk/add_on/scriptgrid/scriptgrid.cpp
	angelscript/sdk/add_on/scriptgrid/scriptgrid.h
	angelscript/sdk/add_on/scripthandle/scripthandle.cpp
	angelscript/sdk/add_on/scripthandle/scripthandle.h
	angelscript/sdk/add_on/scripthelper/scripthelper.cpp
	angelscript/sdk/add_on/scripthelper/scripthelper.h
	angelscript/sdk/add_on/scriptmath/scriptmath.cpp
	angelscript/sdk/add_on/scriptmath/scriptmath.h
	angelscript/sdk/add_on/scriptmath/scriptmathcomplex.cpp
	angelscript/sdk/add_on/scriptmath/scriptmathcomplex.h
	angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
	angelscript/sdk/add_on/scriptstdstring/scriptstdstring.h
	angelscript/sdk/add_on/scriptstdstring/scriptstdstring_utils.cpp
	angelscript/sdk/add_on/serializer/serializer.cpp
	angelscript/sdk/add_on/serializer/serializer.h
	angelscript/sdk/add_on/weakref/weakref.cpp
	angelscript/sdk/add_on/weakref/weakref.h
)

set(SOURCE_FILES 
	src/main.cpp
	src/types.h
	
	# command line
	src/cli/CommandLine.h	src/cli/CommandLine.cpp
	src/cli/ProgressMeter.h	src/cli/ProgressMeter.cpp
	
	# BSP and related structures
	src/bsp/BspMerger.h		src/bsp/BspMerger.cpp
	src/bsp/Bsp.h			src/bsp/Bsp.cpp
	src/bsp/bsplimits.h
	src/bsp/bsptypes.h		src/bsp/bsptypes.cpp
	src/bsp/Entity.h		src/bsp/Entity.cpp
	src/bsp/Keyvalue.h		src/bsp/Keyvalue.cpp
	src/bsp/Wad.h			src/bsp/Wad.cpp
	src/bsp/remap.h			src/bsp/remap.cpp
	src/bsp/colors.h		src/bsp/colors.cpp
	
	# Math and stuff
	src/util/util.h				src/util/util.cpp
	src/util/vectors.h			src/util/vectors.cpp
	src/util/mat4x4.h			src/util/mat4x4.cpp
	src/util/Polygon3D.h		src/util/Polygon3D.cpp
	src/util/Line2D.h			src/util/Line2D.cpp
	src/util/mstream.h			src/util/mstream.cpp
	src/util/ThreadSafeInt.h	src/util/ThreadSafeInt.cpp
	src/util/bmp.h				src/util/bmp.cpp
	src/globals.h				src/globals.cpp
	
	# Navigation meshes
	src/nav/NavMesh.h				src/nav/NavMesh.cpp
	src/nav/NavMeshGenerator.h		src/nav/NavMeshGenerator.cpp
	src/nav/LeafNavMeshGenerator.h	src/nav/LeafNavMeshGenerator.cpp
	src/nav/LeafNavMesh.h			src/nav/LeafNavMesh.cpp
	src/nav/PolyOctree.h			src/nav/PolyOctree.cpp
	src/nav/LeafOctree.h			src/nav/LeafOctree.cpp
	
	# OpenGL rendering
	src/gl/primitives.h			src/gl/primitives.cpp
	src/gl/Shader.h				src/gl/Shader.cpp
	src/gl/ShaderProgram.h		src/gl/ShaderProgram.cpp
	src/gl/VertexBuffer.h		src/gl/VertexBuffer.cpp
	src/gl/Texture.h			src/gl/Texture.cpp
	src/gl/TextureArray.h			src/gl/TextureArray.cpp
	
	# 3D editor
	src/editor/Renderer.h			src/editor/Renderer.cpp
	src/editor/Gui.h				src/editor/Gui.cpp
	src/editor/BspRenderer.h		src/editor/BspRenderer.cpp
	src/editor/PointEntRenderer.h	src/editor/PointEntRenderer.cpp
	src/editor/Fgd.h				src/editor/Fgd.cpp
	src/editor/Clipper.h			src/editor/Clipper.cpp
	src/editor/Command.h			src/editor/Command.cpp
	src/editor/AppSettings.h		src/editor/AppSettings.cpp
	src/editor/MdlRenderer.h		src/editor/MdlRenderer.cpp
	src/editor/SprRenderer.h		src/editor/SprRenderer.cpp
	src/editor/BaseRenderer.h		src/editor/BaseRenderer.cpp
	src/editor/TextureAtlas.h		src/editor/TextureAtlas.cpp
	src/editor/studio.h
	
	# map compiler code
	src/qtools/rad.h		src/qtools/rad.cpp
	src/qtools/vis.h		src/qtools/vis.cpp
	src/qtools/winding.h	src/qtools/winding.cpp
	
	# library files
	imgui/imgui.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_demo.cpp
	imgui/backends/imgui_impl_glfw.cpp
	imgui/backends/imgui_impl_opengl2.cpp
	src/util/lodepng.h
	src/util/lodepng.cpp
	src/util/tinyfiledialogs.c
	src/util/tinyfiledialogs.h
	src/util/lzma_util.h src/util/lzma_util.cpp
	src/util/quant.h src/util/quant.cpp
	
	# AngelScript add-ons
	${ANGELSCRIPT_ADDON_FILES}
)

set(SHADER_FILES
	src/shaders/vec3_vert.glsl
	src/shaders/vec3_frag.glsl
	src/shaders/vec3_depth_frag.glsl
	
	src/shaders/cvert_vert.glsl
	src/shaders/cvert_frag.glsl
	
	src/shaders/bsp_vert.glsl
	src/shaders/bsp_legacy_frag.glsl
	src/shaders/bsp_3dtex_frag.glsl
	src/shaders/bsp_arraytex_frag.glsl
	
	src/shaders/spr_vert.glsl
	src/shaders/spr_frag.glsl
	
	src/shaders/mdl_vert.glsl
	src/shaders/mdl_frag.glsl
	src/shaders/mdl_legacy_vert.glsl
)

# Custom command that always runs because cmake doesn't care if .glsl contents change.
# The command will check if the generated file needs updating or not
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file.h  # fake file that never exists, causing this step to run every build
	${CMAKE_BINARY_DIR}/embedded_shaders.cpp
	COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/embed_shaders.cmake" "embedded_shaders.h" "embedded_shaders.cpp" ${SHADER_FILES}
)
add_custom_target(
	my_custom_target_that_always_runs ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file.h
)

set(SOURCE_FILES ${SOURCE_FILES} "${CMAKE_BINARY_DIR}/embedded_shaders.cpp")

include_directories(src)
include_directories(src/bsp)
include_directories(src/cli)
include_directories(src/data)
include_directories(src/editor)
include_directories(src/gl)
include_directories(src/qtools)
include_directories(src/util)
include_directories(src/nav)
include_directories(src/lib)
include_directories(imgui)
include_directories(imgui/examples)
include_directories(imgui/backends)
include_directories(glew/include)
include_directories(${CMAKE_BINARY_DIR})

add_definitions(-DGLEW_STATIC)

if(MSVC)
	add_executable(${PROJECT_NAME} ${SOURCE_FILES} src/data/icons/app_icon.rc)
	target_link_libraries(${PROJECT_NAME} glfw liblzma angelscript)

	add_custom_command(
		TARGET ${PROJECT_NAME}
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/embed_shaders.cmake" "embedded_shaders.h" "embedded_shaders.cpp" "${SHADER_FILES}"
		${CMAKE_CURRENT_BINARY_DIR}/__dummy_file.h
	)
	add_dependencies(${PROJECT_NAME} my_custom_target_that_always_runs)

	# no warnings for release builds
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W0")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W1")

	# parallel compile,
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /we4715")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /MP /we4715")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_MODE")
	
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE")
	
		
	add_subdirectory(glfw)
	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT bspguy)
	
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(GLEW_PATH ${CMAKE_CURRENT_SOURCE_DIR}/glew/lib/Release/Win32/glew32s.lib)
	else()
		set(GLEW_PATH ${CMAKE_CURRENT_SOURCE_DIR}/glew/lib/Release/x64/glew32s.lib)
	endif()
	
	target_link_libraries(${PROJECT_NAME} opengl32 ${GLEW_PATH})
	
	source_group("Header Files\\bsp" FILES	src/bsp/BspMerger.h
											src/bsp/Bsp.h
											src/bsp/bsplimits.h
											src/bsp/bsptypes.h
											src/bsp/Entity.h
											src/bsp/Keyvalue.h
											src/bsp/Wad.h
											src/bsp/colors.h
											src/bsp/remap.h)
											
	source_group("Source Files\\bsp" FILES	src/bsp/BspMerger.cpp
											src/bsp/Bsp.cpp
											src/bsp/bsptypes.cpp
											src/bsp/Entity.cpp
											src/bsp/Keyvalue.cpp
											src/bsp/Wad.cpp
											src/bsp/colors.cpp
											src/bsp/remap.cpp)
	
	source_group("Header Files\\cli" FILES	src/cli/CommandLine.h
											src/cli/ProgressMeter.h)
											
	source_group("Source Files\\cli" FILES	src/cli/CommandLine.cpp
											src/cli/ProgressMeter.cpp)
	
	source_group("Header Files\\gl" FILES	src/gl/Shader.h
											src/gl/ShaderProgram.h
											src/gl/VertexBuffer.h
											src/gl/Texture.h
											src/gl/TextureArray.h
											src/gl/primitives.h
											${CMAKE_BINARY_DIR}/embedded_shaders.h
											src/gl/shaders.h)
											
	source_group("Source Files\\gl" FILES	src/gl/Shader.cpp
											src/gl/ShaderProgram.cpp
											src/gl/VertexBuffer.cpp
											src/gl/Texture.cpp
											src/gl/TextureArray.cpp
											src/gl/primitives.cpp
											${CMAKE_BINARY_DIR}/embedded_shaders.cpp
											src/gl/shaders.cpp)
											
	source_group("Source Files\\gl\\shaders" FILES ${SHADER_FILES})
											
	source_group("Header Files\\editor" FILES	src/editor/BspRenderer.h
												src/editor/TextureAtlas.h
												src/editor/Renderer.h
												src/editor/Fgd.h
												src/editor/Gui.h
												src/editor/PointEntRenderer.h
												src/editor/Command.h
												src/editor/AppSettings.h
												src/editor/MdlRenderer.h
												src/editor/SprRenderer.h
												src/editor/BaseRenderer.h
												src/editor/Clipper.h)
											
	source_group("Source Files\\editor" FILES	src/editor/BspRenderer.cpp
												src/editor/TextureAtlas.cpp
												src/editor/Renderer.cpp
												src/editor/Fgd.cpp
												src/editor/Gui.cpp
												src/editor/PointEntRenderer.cpp
												src/editor/Command.cpp
												src/editor/AppSettings.cpp
												src/editor/MdlRenderer.cpp
												src/editor/SprRenderer.cpp
												src/editor/BaseRenderer.cpp
												src/editor/Clipper.cpp)
											
	source_group("Header Files\\qtools" FILES	src/qtools/rad.h
												src/qtools/vis.h
												src/qtools/winding.h)
												
	source_group("Source Files\\qtools" FILES	src/qtools/rad.cpp
												src/qtools/vis.cpp
												src/qtools/winding.cpp)
												
	source_group("Header Files\\util" FILES		src/util/util.h
												src/util/vectors.h
												src/util/Polygon3D.h
												src/util/Line2D.h
												src/util/mstream.h
												src/util/lzma_util.h
												src/util/ThreadSafeInt.h
												src/util/mat4x4.h
												src/util/bmp.h)
												
	source_group("Source Files\\util" FILES		src/util/util.cpp
												src/util/vectors.cpp
												src/util/Polygon3D.cpp
												src/util/Line2D.cpp
												src/util/mstream.cpp
												src/util/lzma_util.cpp
												src/util/ThreadSafeInt.cpp
												src/util/mat4x4.cpp
												src/util/bmp.cpp)
												
	source_group("Header Files\\nav" FILES		src/nav/NavMesh.h
												src/nav/NavMeshGenerator.h
												src/nav/LeafNavMeshGenerator.h
												src/nav/LeafNavMesh.h
												src/nav/PolyOctree.h
												src/nav/LeafOctree.h)
												
	source_group("Source Files\\nav" FILES		src/nav/NavMesh.cpp
												src/nav/NavMeshGenerator.cpp
												src/nav/LeafNavMeshGenerator.cpp
												src/nav/LeafNavMesh.cpp
												src/nav/PolyOctree.cpp
												src/nav/LeafOctree.cpp)
	
	source_group("Header Files\\util\\lib" FILES	src/util/lodepng.h)
	
	source_group("Source Files\\util\\lib" FILES	imgui/imgui.cpp
													imgui/imgui_tables.cpp
													imgui/imgui_widgets.cpp
													imgui/imgui_draw.cpp
													imgui/imgui_demo.cpp
													imgui/backends/imgui_impl_glfw.cpp
													imgui/backends/imgui_impl_opengl2.cpp
													src/util/lodepng.cpp)
	
	# AngelScript add-ons source groups
	source_group("Source Files\\angelscript\\contextmgr" FILES
													angelscript/sdk/add_on/contextmgr/contextmgr.cpp
													angelscript/sdk/add_on/contextmgr/contextmgr.h)
	source_group("Source Files\\angelscript\\datetime" FILES
													angelscript/sdk/add_on/datetime/datetime.cpp
													angelscript/sdk/add_on/datetime/datetime.h)
	source_group("Source Files\\angelscript\\debugger" FILES
													angelscript/sdk/add_on/debugger/debugger.cpp
													angelscript/sdk/add_on/debugger/debugger.h)
	source_group("Source Files\\angelscript\\scriptany" FILES
													angelscript/sdk/add_on/scriptany/scriptany.cpp
													angelscript/sdk/add_on/scriptany/scriptany.h)
	source_group("Source Files\\angelscript\\scriptarray" FILES
													angelscript/sdk/add_on/scriptarray/scriptarray.cpp
													angelscript/sdk/add_on/scriptarray/scriptarray.h)
	source_group("Source Files\\angelscript\\scriptbuilder" FILES
													angelscript/sdk/add_on/scriptbuilder/scriptbuilder.cpp
													angelscript/sdk/add_on/scriptbuilder/scriptbuilder.h)
	source_group("Source Files\\angelscript\\scriptdictionary" FILES
													angelscript/sdk/add_on/scriptdictionary/scriptdictionary.cpp
													angelscript/sdk/add_on/scriptdictionary/scriptdictionary.h)
	source_group("Source Files\\angelscript\\scriptfile" FILES
													angelscript/sdk/add_on/scriptfile/scriptfile.cpp
													angelscript/sdk/add_on/scriptfile/scriptfile.h
													angelscript/sdk/add_on/scriptfile/scriptfilesystem.cpp
													angelscript/sdk/add_on/scriptfile/scriptfilesystem.h)
	source_group("Source Files\\angelscript\\scriptgrid" FILES
													angelscript/sdk/add_on/scriptgrid/scriptgrid.cpp
													angelscript/sdk/add_on/scriptgrid/scriptgrid.h)
	source_group("Source Files\\angelscript\\scripthandle" FILES
													angelscript/sdk/add_on/scripthandle/scripthandle.cpp
													angelscript/sdk/add_on/scripthandle/scripthandle.h)
	source_group("Source Files\\angelscript\\scripthelper" FILES
													angelscript/sdk/add_on/scripthelper/scripthelper.cpp
													angelscript/sdk/add_on/scripthelper/scripthelper.h)
	source_group("Source Files\\angelscript\\scriptmath" FILES
													angelscript/sdk/add_on/scriptmath/scriptmath.cpp
													angelscript/sdk/add_on/scriptmath/scriptmath.h
													angelscript/sdk/add_on/scriptmath/scriptmathcomplex.cpp
													angelscript/sdk/add_on/scriptmath/scriptmathcomplex.h)
	source_group("Source Files\\angelscript\\scriptstdstring" FILES
													angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
													angelscript/sdk/add_on/scriptstdstring/scriptstdstring.h
													angelscript/sdk/add_on/scriptstdstring/scriptstdstring_utils.cpp)
	source_group("Source Files\\angelscript\\serializer" FILES
													angelscript/sdk/add_on/serializer/serializer.cpp
													angelscript/sdk/add_on/serializer/serializer.h)
	source_group("Source Files\\angelscript\\weakref" FILES
													angelscript/sdk/add_on/weakref/weakref.cpp
													angelscript/sdk/add_on/weakref/weakref.h)

else()
	add_executable(${PROJECT_NAME} ${SOURCE_FILES})
	target_link_libraries(${PROJECT_NAME} glfw liblzma angelscript GL GLU X11 Xxf86vm Xrandr pthread Xi GLEW stdc++fs ${CMAKE_DL_LIBS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -fno-omit-frame-pointer -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_MODE -g -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()
