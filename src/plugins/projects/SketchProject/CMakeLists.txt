# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(SKETCH_PROJECT_SRC
        SketchProjectMainHeader.cpp
        SketchProjectModule.cpp
        SketchProject.cpp
        SketchProjectDesigner.cpp
        SketchProjectWindow.cpp
        SketchProjectWindowEdit.cpp
        SketchProjectWindowStructure.cpp
        SketchProjectJSObject.cpp
        )

# program windows --------------------------------------------------------------
SET(SKETCH_PROJECT_UIS
        SketchProjectWindow.ui
        SketchProjectDesigner.ui
        )

# program resources ------------------------------------------------------------
SET(SKETCH_PROJECT_RESOURCES
        SketchProject.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(SKETCH_PROJECT_SRC ${SKETCH_PROJECT_RESOURCES})
QT5_WRAP_UI(SKETCH_PROJECT_SRC ${SKETCH_PROJECT_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(sketch_project_shared SHARED ${SKETCH_PROJECT_SRC})
ADD_DEPENDENCIES(sketch_project_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(sketch_project_shared PROPERTIES
                        OUTPUT_NAME SketchProject
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(sketch_project_shared
                ${INDIGO_LIB}
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL Qt5::Svg Qt5::WebEngineWidgets
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            sketch_project_shared
        DESTINATION
            lib
        )

