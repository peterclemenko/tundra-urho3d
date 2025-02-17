# Define target name and output directory
init_target(TundraLogic OUTPUT Plugins)

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

add_definitions(-DTUNDRALOGIC_EXPORTS)

UseTundraCore()
use_modules(TundraCore Plugins/UrhoRenderer Plugins/BulletPhysics)
use_package(BULLET)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

link_modules(TundraCore UrhoRenderer BulletPhysics)
link_package(URHO3D)
link_package(MATHGEOLIB)
link_package(KNET)

SetupCompileFlagsWithPCH()

final_target()
