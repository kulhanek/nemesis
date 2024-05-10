# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(TURBOMOLE_SRC
        TurbomoleMainHeader.cpp
        TurbomoleModule.cpp
        CoordImportTool.cpp
        CoordImportJob.cpp
        TVibTrajSegment.cpp
        TVibTrajImportTool.cpp
        )

# program windows --------------------------------------------------------------
SET(TURBOMOLE_UIS
        CoordImportTool.ui
        TVibTrajImportTool.ui
        )

# program resources ------------------------------------------------------------
SET(TURBOMOLE_RESOURCES
        TurbomoleModule.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(TURBOMOLE_SRC ${TURBOMOLE_RESOURCES})
QT5_WRAP_UI(TURBOMOLE_SRC ${TURBOMOLE_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(turbomole_shared SHARED ${TURBOMOLE_SRC})
ADD_DEPENDENCIES(turbomole_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(turbomole_shared PROPERTIES
                        OUTPUT_NAME Turbomole
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(turbomole_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                ${EXTRA_IMPEX_LIB}
                ${OPEN_BABEL_LIB}
                )

INSTALL(TARGETS
            turbomole_shared
        DESTINATION
            lib
        )

