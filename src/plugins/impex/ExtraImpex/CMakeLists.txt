# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(EXTRA_IMPEX_SRC
        ExtraImpexMainHeader.cpp
        ExtraImpexModule.cpp

    # mol2 format --------------------------------
        Mol2ImportTool.cpp
        Mol2ImportJob.cpp
        Mol2ExportTool.cpp
        Mol2ExportJob.cpp
        Mol2ImportTypes.cpp

    # pdb format ---------------------------------
        PdbExportTool.cpp
        PdbExportJob.cpp

    # pdbqt format -------------------------------
        #PdbqtExportTool.cpp
        #PdbqtExportJob.cpp

    # nstr format --------------------------------
        NStrExportTool.cpp
        NStrExportJob.cpp

    # xyz format ---------------------------------
        XYZImportTool.cpp
        XYZImportJob.cpp
        XYZTrajImportTool.cpp
        XYZExportTool.cpp
        XYZExportJob.cpp
        )

# program windows --------------------------------------------------------------
SET(EXTRA_IMPEX_UIS
        )

# QT5 related binding ----------------------------------------------------------
QT5_WRAP_UI(EXTRA_IMPEX_SRC ${EXTRA_IMPEX_UIS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_LIBRARY(extra_impex_shared SHARED ${EXTRA_IMPEX_SRC})
ADD_DEPENDENCIES(extra_impex_shared nemesis_core_shared)

SET_TARGET_PROPERTIES(extra_impex_shared PROPERTIES
                        OUTPUT_NAME ExtraImpex
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

TARGET_LINK_LIBRARIES(extra_impex_shared
                NemesisCore
                plusula
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                ${OPEN_BABEL_LIB}
                )

INSTALL(TARGETS
            extra_impex_shared
        DESTINATION
            lib
        )

