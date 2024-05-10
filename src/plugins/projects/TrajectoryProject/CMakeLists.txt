# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(TRAJECTORY_PROJECT_SRC
        TrajectoryProjectMainHeader.cpp
        TrajectoryProjectModule.cpp
        TrajectoryProject.cpp
        TrajectoryProjectDesigner.cpp
        TrajectoryProjectWindow.cpp
        TrajectoryProjectWindowTrajectory.cpp
        )

# program windows --------------------------------------------------------------
SET(TRAJECTORY_PROJECT_UIS
        TrajectoryProjectDesigner.ui
        TrajectoryProjectWindow.ui
        )

# program resources ------------------------------------------------------------
SET(TRAJECTORY_PROJECT_RESOURCES
        TrajectoryProject.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(TRAJECTORY_PROJECT_SRC ${TRAJECTORY_PROJECT_RESOURCES})
QT5_WRAP_UI(TRAJECTORY_PROJECT_SRC ${TRAJECTORY_PROJECT_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(trajectory_project_shared SHARED ${TRAJECTORY_PROJECT_SRC})
ADD_DEPENDENCIES(trajectory_project_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(trajectory_project_shared PROPERTIES
                        OUTPUT_NAME TrajectoryProject
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(trajectory_project_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            trajectory_project_shared
        DESTINATION
            lib
        )

