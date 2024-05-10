# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(RMSDTOOLS_SRC
        RMSDToolsMainHeader.cpp
        RMSDToolsModule.cpp
        MapCouple.cpp
        MapCoupleHistory.cpp
        RMSDProperty.cpp
        RMSDPropertyHistory.cpp
        RMSDPropertyModel.cpp
        RMSDPropertyDesigner.cpp
        RMSDPropertySetup.cpp
        RMSDPropertySetupDesigner.cpp
        )

# program windows --------------------------------------------------------------
SET(RMSDTOOLS_UIS
        RMSDPropertyDesigner.ui
        RMSDPropertySetupDesigner.ui
        )

# program resources ------------------------------------------------------------
SET(RMSDTOOLS_RESOURCES
        RMSDToolsModule.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_ADD_RESOURCES(RMSDTOOLS_SRC ${RMSDTOOLS_RESOURCES})
QT5_WRAP_UI(RMSDTOOLS_SRC ${RMSDTOOLS_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(rmsdtools_shared SHARED ${RMSDTOOLS_SRC})
ADD_DEPENDENCIES(rmsdtools_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(rmsdtools_shared PROPERTIES
                        OUTPUT_NAME RMSDTools
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(rmsdtools_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL
                ${OPENGL_glu_LIBRARY}
                ${OPENGL_gl_LIBRARY}
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            rmsdtools_shared
        DESTINATION
            lib
        )

