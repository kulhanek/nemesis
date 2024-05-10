# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(STANDARD_WP_SRC
        StandardWPMainHeader.cpp
        StandardWPModule.cpp
        BuildWorkPanel.cpp
        DockingWorkPanel.cpp
        GeometryWorkPanel.cpp
        GeoManipWorkPanel.cpp
        SelectionWorkPanel.cpp
        JobWorkPanel.cpp
        NewGraphicsObjectWorkPanel.cpp
        GraphicsManagerWorkPanel.cpp
        ActiveProfileManagerWorkPanel.cpp
        RestraintsManagerWorkPanel.cpp
        SelectByMaskWorkPanel.cpp
        NewPropertyWorkPanel.cpp
        PropertyManagerWorkPanel.cpp
        StructureListWorkPanel.cpp
        TrajectoryListWorkPanel.cpp
        NewSnapshotFilterWorkPanel.cpp
        )

# program windows --------------------------------------------------------------
SET(STANDARD_WP_UIS
        BuildWorkPanel.ui
        DockingWorkPanel.ui
        GeometryWorkPanel.ui
        GeoManipWorkPanel.ui
        SelectionWorkPanel.ui
        JobWorkPanel.ui
        NewGraphicsObjectWorkPanel.ui
        GraphicsManagerWorkPanel.ui
        ActiveProfileManagerWorkPanel.ui
        RestraintsManagerWorkPanel.ui
        SelectByMaskWorkPanel.ui
        NewPropertyWorkPanel.ui
        PropertyManagerWorkPanel.ui
        StructureListWorkPanel.ui
        TrajectoryListWorkPanel.ui
        NewSnapshotFilterWorkPanel.ui
        )

# program resources
SET(STANDARD_WP_RESOURCES
        StandardWP.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_WRAP_UI(STANDARD_WP_SRC ${STANDARD_WP_UIS})
QT5_ADD_RESOURCES(STANDARD_WP_SRC ${STANDARD_WP_RESOURCES})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(standard_wp SHARED ${STANDARD_WP_SRC})
ADD_DEPENDENCIES(standard_wp nemesis_core_shared)

SET_TARGET_PROPERTIES(standard_wp PROPERTIES
                        OUTPUT_NAME StandardWP
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(standard_wp
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            standard_wp
        DESTINATION
            lib
        )

