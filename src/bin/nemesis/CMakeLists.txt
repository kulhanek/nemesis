# ==============================================================================
# NEMESIS CMake File
# ==============================================================================

# we want GUI application under windows
IF(WIN32)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mwindows")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mwindows")
ENDIF(WIN32)

# program objects --------------------------------------------------------------
SET(NEMESIS_SRC
        NemesisApplication.cpp
        main.cpp
        )

# add program icon under win32/mingw -------------------------------------------
if(WIN32)
     set(RES_FILES "nemesis.rc")
     SET(NEMESIS_SRC ${NEMESIS_SRC} ${RES_FILES})
endif(WIN32)

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_nemesis.exe ${NEMESIS_SRC})

ADD_DEPENDENCIES(_nemesis.exe nemesis_core_shared)

TARGET_LINK_LIBRARIES(_nemesis.exe
                NemesisCore
                Qt5::Core Qt5::Widgets Qt5::Script
                ${PLUSULA_LIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

INSTALL(TARGETS
            _nemesis.exe
        RUNTIME DESTINATION
            bin
        )
