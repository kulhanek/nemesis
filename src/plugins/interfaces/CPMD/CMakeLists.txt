# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(CPMD_SRC
        CPMDMainHeader.cpp
        CPMDModule.cpp
        CPMDInputExportTool.cpp
        )

# program windows --------------------------------------------------------------
SET(CPMD_UIS
        CPMDInputExportTool.ui
        )

# program resources ------------------------------------------------------------
SET(CPMD_RESOURCES
        CPMDModule.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(CPMD_SRC ${CPMD_RESOURCES})
QT5_WRAP_UI(CPMD_SRC ${CPMD_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(cpmd_shared SHARED ${CPMD_SRC})
ADD_DEPENDENCIES(cpmd_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(cpmd_shared PROPERTIES
                        OUTPUT_NAME CPMD
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(cpmd_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                ${EXTRA_IMPEX_LIB}
                )

INSTALL(TARGETS
            cpmd_shared
        DESTINATION
            lib
        )

