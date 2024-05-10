# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(AMBER_PLUGIN_SRC
        AmberMainHeader.cpp
        AmberModule.cpp
        AmberImportJob.cpp
        AmberImportTool.cpp
        Rst7ExportJob.cpp
        Rst7ExportTool.cpp
        TopologyImportJob.cpp
        TopologyImportTool.cpp
        ChargesImportJob.cpp
        ChargesImportTool.cpp
        GESPImportJob.cpp
        GESPImportTool.cpp
        GESPGridObject.cpp
        GESPGridObjectDesigner.cpp
        GESPGridSetup.cpp
        GESPGridSetupDesigner.cpp
        GESPAsRESPImportTool.cpp
        GESPAsRESPImportJob.cpp
        )

# program windows --------------------------------------------------------------
SET(AMBER_PLUGIN_UIS
        AmberImportTool.ui
        Rst7ExportTool.ui
        GESPImportTool.ui
        GESPGridObjectDesigner.ui
        GESPGridSetupDesigner.ui
        GESPAsRESPImportTool.ui
        )

# program resources ------------------------------------------------------------
SET(AMBER_PLUGIN_RESOURCES
        AmberModule.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(AMBER_PLUGIN_SRC ${AMBER_PLUGIN_RESOURCES})
QT5_WRAP_UI(AMBER_PLUGIN_SRC ${AMBER_PLUGIN_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(amber_plugin_shared SHARED ${AMBER_PLUGIN_SRC})
ADD_DEPENDENCIES(amber_plugin_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(amber_plugin_shared PROPERTIES
                        OUTPUT_NAME AmberPlugin
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(amber_plugin_shared
                ${NLEAP_CMDS_LIB}
                ${NLEAP_LIB}
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL
                ${PLUSULA_LIB_NAME}
                ${ASL_LIB_NAME}
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            amber_plugin_shared
        DESTINATION
            lib
        )
