# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(BUILD_PROJECT_SRC
        BuildProjectMainHeader.cpp
        BuildProjectModule.cpp
        BuildProject.cpp
        BuildProjectDesigner.cpp
        BuildProjectWindow.cpp
        )

# program windows --------------------------------------------------------------
SET(BUILD_PROJECT_UIS
        BuildProjectWindow.ui
        BuildProjectDesigner.ui
        )

# program resources ------------------------------------------------------------
SET(BUILD_PROJECT_RESOURCES
        BuildProject.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(BUILD_PROJECT_SRC ${BUILD_PROJECT_RESOURCES})
QT5_WRAP_UI(BUILD_PROJECT_SRC ${BUILD_PROJECT_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(build_project_shared SHARED ${BUILD_PROJECT_SRC})
ADD_DEPENDENCIES(build_project_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(build_project_shared PROPERTIES
                        OUTPUT_NAME BuildProject
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(build_project_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            build_project_shared
        DESTINATION
            lib
        )

