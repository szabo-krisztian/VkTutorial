add_library(Utils utilities.cpp)
target_link_libraries(Utils PUBLIC PHYSX)

add_library(SimulatorBase simulator_base.cpp)
target_link_libraries(SimulatorBase
    PUBLIC PHYSX
)

add_library(Simulator simulator.cpp)
target_link_libraries(Simulator
    PUBLIC SimulatorBase
           Utils
)