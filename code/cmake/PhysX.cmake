if (NOT DEFINED PHYSX_INCLUDE)
    message(FATAL_ERROR "PhysX include path not defined!")
endif()

if (NOT DEFINED PHYSX_LIB)
    message(FATAL_ERROR "PhysX lib path not defined!")
endif()

add_library(PHYSX INTERFACE)
target_include_directories(PHYSX INTERFACE ${PHYSX_INCLUDE})
target_link_directories(PHYSX INTERFACE ${PHYSX_LIB})
target_link_libraries(PHYSX
    INTERFACE
        PhysXExtensions_static_64
        PhysX_64
        PhysXPvdSDK_static_64
        PhysXCommon_64
        PhysXFoundation_64
        PhysXCooking_64
)