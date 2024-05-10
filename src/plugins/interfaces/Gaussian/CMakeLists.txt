# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(GAUSSIAN_SRC
        GaussianMainHeader.cpp
        GaussianModule.cpp
        GaussianInputExportTool.cpp
        GOptTrajSegment.cpp
        GOptTrajSegmentDesigner.cpp
        GOptTrajImportTool.cpp
        GDrvTrajSegment.cpp
        GDrvTrajSegmentDesigner.cpp
        GDrvTrajImportTool.cpp
        GaussianUtils.cpp
        GVibTrajImportTool.cpp
        GVibTrajSegment.cpp
        )

# program windows --------------------------------------------------------------
SET(GAUSSIAN_UIS
        GaussianInputExportTool.ui
        GOptTrajSegmentDesigner.ui
        GDrvTrajSegmentDesigner.ui
        )

# program resources ------------------------------------------------------------
SET(GAUSSIAN_RESOURCES
        GaussianModule.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(GAUSSIAN_SRC ${GAUSSIAN_RESOURCES})
QT5_WRAP_UI(GAUSSIAN_SRC ${GAUSSIAN_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(gaussian_shared SHARED ${GAUSSIAN_SRC})
ADD_DEPENDENCIES(gaussian_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(gaussian_shared PROPERTIES
                        OUTPUT_NAME Gaussian
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(gaussian_shared
                NemesisCore
                plusula
                ${QCP_LIB}
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                ${EXTRA_IMPEX_LIB}
                ${OPEN_BABEL_LIB}
                )

INSTALL(TARGETS
            gaussian_shared
        DESTINATION
            lib
        )

