# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(EXTRA_WP_SRC
        AutoRotateWorkPanel.cpp
        CompareInternalGeometry.cpp
        ExtraWPMainHeader.cpp
        ExtraWPModule.cpp
        ResidueListWorkPanel.cpp
        ResidueWorkPanel.cpp
        StructureInternalGeometry.cpp
        StructurePaletteWorkPanel.cpp
        SuperCellBuilderWorkPanel.cpp
        )

# program windows --------------------------------------------------------------
SET(EXTRA_WP_UIS
        AutoRotateWorkPanel.ui
        CompareInternalGeometry.ui
        ResidueListWorkPanel.ui
        ResidueWorkPanel.ui
        StructureInternalGeometry.ui
        StructurePaletteWorkPanel.ui
        SuperCellBuilderWorkPanel.ui
        )

# program resources ------------------------------------------------------------
SET(EXTRA_WP_RESOURCES
        ExtraWP.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_WRAP_UI(EXTRA_WP_SRC ${EXTRA_WP_UIS})
QT5_ADD_RESOURCES(EXTRA_WP_SRC ${EXTRA_WP_RESOURCES})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(extra_wp_shared SHARED ${EXTRA_WP_SRC})
ADD_DEPENDENCIES(extra_wp_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(extra_wp_shared PROPERTIES
                        OUTPUT_NAME ExtraWP
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(extra_wp_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                ${EXTRA_WP_LIB}
                )

INSTALL(TARGETS
            extra_wp_shared
        DESTINATION
            lib
        )

