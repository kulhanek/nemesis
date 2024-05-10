# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(OPEN_BABEL_OBJ
        OpenBabelMainHeader.cpp
        OpenBabelModule.cpp
        OpenBabelExportTool.cpp
        OpenBabelImportTool.cpp
        OpenBabelImportJob.cpp
        OpenBabelOptimizerSetup.cpp
        OpenBabelOptimizerSetupDesigner.cpp
        OpenBabelOptimizer.cpp
        OpenBabelImpexFormats.cpp
        GraphSymmetryWorkPanel.cpp
        AutomorphismWorkPanel.cpp
        )

# program windows --------------------------------------------------------------
SET(OPEN_BABEL_UIS
        OpenBabelOptimizerSetupDesigner.ui
        GraphSymmetryWorkPanel.ui
        AutomorphismWorkPanel.ui
        )

# program resources ------------------------------------------------------------
SET(OPEN_BABEL_RESOURCES
        OpenBabel.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_WRAP_UI(OPEN_BABEL_OBJ ${OPEN_BABEL_UIS})
QT5_ADD_RESOURCES(OPEN_BABEL_OBJ ${OPEN_BABEL_RESOURCES})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(open_babel_shared SHARED ${OPEN_BABEL_OBJ})
ADD_DEPENDENCIES(open_babel_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(open_babel_shared PROPERTIES
                        OUTPUT_NAME OpenBabel
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(open_babel_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                ${OPEN_BABEL_LIB}
                )

INSTALL(TARGETS
            open_babel_shared
        DESTINATION
            lib
        )

