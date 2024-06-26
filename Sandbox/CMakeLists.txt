cmake_minimum_required(VERSION 3.21.0)
project(CapitalPunishment VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
	src/ECS/Entity.h
	src/ECS/Entity.cpp
	src/ECS/EntityImpl.h
	src/ECS/Scene.h
	src/ECS/Scene.cpp

	src/ECS/Components/Components.h
	src/ECS/Components/Components.cpp

	src/ECS/System.h
	src/ECS/System.cpp

	src/ECS/Components/RenderableComponents.h
	src/ECS/Components/RenderableComponents.cpp

	src/Scripts/ScriptableEntity.h
	src/Scripts/ScriptableEntity.cpp
	src/Scripts/ScriptEvents.h
	src/Scripts/ScriptEvents.cpp
	src/Scripts/NativeScript.h
	src/Scripts/NativeScript.cpp

	src/Scripts/Process.h
	src/Scripts/Process.cpp

	src/SceneGraph/SceneGraph.h
	src/SceneGraph/SceneGraph.cpp

	src/RenderScene/RenderScene.h
	src/RenderScene/RenderScene.cpp

	src/Physics/PhysicsTypes.h
	src/Physics/PhysicsTypes.cpp
	src/Physics/Collider.h
	src/Physics/Collider.cpp
	src/Physics/PhysicsEvents.h
	src/Physics/PhysicsEvents.cpp
	src/Physics/Physics3D.h
	src/Physics/Physics3D.cpp

	src/ParticleSystem/Particles.h
	src/ParticleSystem/Particles.cpp

	src/CapitalPunishment.h
	src/CapitalPunishment.cpp

	src/Scripts/CameraController.h
	src/Scripts/CameraController.cpp

	src/Scripts/Sun.h
	src/Scripts/Sun.cpp

	src/Scripts/Unit.h
	src/Scripts/Unit.cpp
	src/Scripts/UnitController.h
	src/Scripts/UnitController.cpp

	src/Scripts/Enemy.h
	src/Scripts/Enemy.cpp

	src/Scripts/Effect.h
	src/Scripts/Effect.cpp

	src/Scripts/Projectile.h
	src/Scripts/Projectile.cpp

	src/Scripts/Turret.h
	src/Scripts/Turret.cpp

	src/Scripts/VillageManager.h
	src/Scripts/VillageManager.cpp
	src/Scripts/Villager.h
	src/Scripts/Villager.cpp
)
add_subdirectory(vendor/entt)

if(true)
    add_compile_definitions(Y_DEBUG)
	target_sources(${PROJECT_NAME} PUBLIC 
		src/Editor/EditorLayer.h
		src/Editor/EditorLayer.cpp
		src/Editor/EditorEvents.h
		src/Editor/EditorEvents.cpp

		src/Editor/Panel/IPanel.h
		src/Editor/Panel/IPanel.cpp

		src/Editor/Panel/ViewportPanel.h
		src/Editor/Panel/ViewportPanel.cpp

		src/Editor/Panel/SceneHierarchyPanel.h
		src/Editor/Panel/SceneHierarchyPanel.cpp

		src/Editor/Panel/InspectorPanel.h
		src/Editor/Panel/InspectorPanel.cpp
	)
endif()

# TODO: Make PhysX library platfrom independent
set(PHYSX_LIB_PATH ${PROJECT_SOURCE_DIR}/vendor/PhysX)
option(PHYSX_BUILD_TYPE "The build type of PhysX, i.e., one of {debug, checked, profile, release}" "debug")

# TODO: Make Physx library detect configuration
if(CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug" OR TRUE)
	if(MSVC)
    	message("Building snippet in [MSVC][debug] configuration")
    	target_link_directories(${PROJECT_NAME} PUBLIC	${PHYSX_LIB_PATH}/lib/debug) # This is the path where PhysX libraries are installed
	elseif(UNIX)
    	message("Building snippet in [UNIX][debug] configuration")
    	link_directories("${PHYSX_LIB_PATH}linux.clang/${PHYSX_BUILD_TYPE}") # This is the path where PhysX libraries are installed
	endif()
else()
	if(MSVC)
    	message("Building snippet in [MSVC][release] configuration")
    	target_link_directories(${PROJECT_NAME} PUBLIC ${PHYSX_LIB_PATH}/lib/release) # This is the path where PhysX libraries are installed
	elseif(UNIX)
    message("Building snippet in release configuration with PhysX ${PHYSX_BUILD_TYPE} configuration")
    link_directories("PhysX/bin/linux.clang/${PHYSX_BUILD_TYPE}") # This is the path where PhysX libraries are installed
	endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src/  ${PHYSX_LIB_PATH}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC 
	YoYo 
	EnTT
	ImGui

	PhysXExtensions_static_64
	PhysXPvdSDK_static_64
	# PhysXCooking_64
	# PhysXGPU_64
	PhysX_64
	PhysXCommon_64
	PhysXFoundation_64
)

add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/assets
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
	COMMENT "Copying resources into bin and binary folder."
	COMMAND cmd /c ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/shaders/compile_shaders.bat
	COMMENT "Compiling shaders.")

# add_dependencies(${PROJECT_NAME} copy_assets)

add_custom_target(copy_physx_binaries ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/vendor/Physx/lib/
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMENT "Copying PhysX binaries into bin and binary folder.")

set_target_properties(
	${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})