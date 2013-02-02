project(facetracknoir)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL QtNetwork QtDesigner REQUIRED)
find_package(OpenCV)
find_package(OpenGL REQUIRED)
find_package(Qxt COMPONENTS QxtGui REQUIRED)

if(NOT QXT_QXTGUI_INCLUDE_DIR OR QXT_QXTGUI_INCLUDE_DIR STREQUAL "QXT_QXTGUI_INCLUDE_DIR-NOTFOUND")
    message(FATAL_ERROR "Qxt not found, required for keybindings, get from http://libqxt.org")
endif()

include_directories(${QXT_QXTGUI_INCLUDE_DIR})
include_directories(${QXT_QXTCORE_INCLUDE_DIR})
if(EXISTS "${QXT_QXTCORE_INCLUDE_DIR}/QxtCore/")
    include_directories(${QXT_QXTCORE_INCLUDE_DIR}/QxtCore)
endif()
if(EXISTS "${QXT_QXTGUI_INCLUDE_DIR}/QxtGui/")
    include_directories(${QXT_QXTGUI_INCLUDE_DIR}/QxtGui)
endif()
INCLUDE_DIRECTORIES(${QT_QTDESIGNER_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

if (WIN32)
    SET(SDK_SM_FACEAPI_PATH "" CACHE STRING "SeeingMachines FaceAPI SDK path")
    set(SDK_MINGW_PATH "c:/mingw" CACHE STRING "MinGW path")
    SET(SDK_MSVC80_PATH "C:/Program Files (x86)/Microsoft Visual Studio 8/VC" CACHE STRING "Path to MSVC80/VC if you want FaceAPI")
    set(SDK_SIMCONNECT "" CACHE STRING "Path to SimConnect SDK")
endif()

if(WIN32)
  set(CMAKE_RC_COMPILER_INIT windres)
  ENABLE_LANGUAGE(RC)
  SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif(WIN32)

# main app

file(GLOB ftnoir-bin-c "facetracknoir/*.cpp" "facetracknoir/*.rc")
file(GLOB ftnoir-bin-h "facetracknoir/*.h")
QT4_WRAP_CPP(ftnoir-bin-moc ${ftnoir-bin-h})
file(GLOB ftnoir-bin-ui "facetracknoir/*.ui")
file(GLOB ftnoir-bin-rc "facetracknoir/*.qrc")
QT4_WRAP_UI(ftnoir-bin-uih ${ftnoir-bin-ui})
QT4_ADD_RESOURCES(ftnoir-bin-rcc ${ftnoir-bin-rc})

file(GLOB ftnoir-pose-widget-c "ftnoir_posewidget/*.cpp")
file(GLOB ftnoir-pose-widget-h "ftnoir_posewidget/*.h")
QT4_WRAP_CPP(ftnoir-pose-widget-moc ${ftnoir-pose-widget-h})
file(GLOB ftnoir-pose-widget-rc "ftnoir_posewidget/*.qrc")
QT4_ADD_RESOURCES(ftnoir-pose-widget-rcc ${ftnoir-pose-widget-rc})

file(GLOB ftnoir-spline-widget-c "qfunctionconfigurator/*.cpp")
file(GLOB ftnoir-spline-widget-h "qfunctionconfigurator/*.h")
QT4_WRAP_CPP(ftnoir-spline-widget-moc ${ftnoir-spline-widget-h})

# filters

file(GLOB ftnoir-filter-accela-c "ftnoir_filter_accela/*.cpp")
file(GLOB ftnoir-filter-accela-h "ftnoir_filter_accela/*.h")
QT4_WRAP_CPP(ftnoir-filter-accela-moc ${ftnoir-filter-accela-h})
file(GLOB ftnoir-filter-accela-ui "ftnoir_filter_accela/*.ui")
file(GLOB ftnoir-filter-accela-rc "ftnoir_filter_accela/*.qrc")
QT4_WRAP_UI(ftnoir-filter-accela-uih ${ftnoir-filter-accela-ui})
QT4_ADD_RESOURCES(ftnoir-filter-accela-rcc ${ftnoir-filter-accela-rc})

file(GLOB ftnoir-filter-ewma-c "ftnoir_filter_ewma2/*.cpp")
file(GLOB ftnoir-filter-ewma-h "ftnoir_filter_ewma2/*.h")
QT4_WRAP_CPP(ftnoir-filter-ewma-moc ${ftnoir-filter-ewma-h})
file(GLOB ftnoir-filter-ewma-ui "ftnoir_filter_ewma2/*.ui")
file(GLOB ftnoir-filter-ewma-rc "ftnoir_filter_ewma2/*.qrc")
QT4_WRAP_UI(ftnoir-filter-ewma-uih ${ftnoir-filter-ewma-ui})
QT4_ADD_RESOURCES(ftnoir-filter-ewma-rcc ${ftnoir-filter-ewma-rc})

# protocols

file(GLOB ftnoir-proto-fgfs-c "ftnoir_protocol_fg/*.cpp")
file(GLOB ftnoir-proto-fgfs-h "ftnoir_protocol_fg/*.h")
QT4_WRAP_CPP(ftnoir-proto-fgfs-moc ${ftnoir-proto-fgfs-h})
file(GLOB ftnoir-proto-fgfs-ui "ftnoir_protocol_fg/*.ui")
file(GLOB ftnoir-proto-fgfs-rc "ftnoir_protocol_fg/*.qrc")
QT4_WRAP_UI(ftnoir-proto-fgfs-uih ${ftnoir-proto-fgfs-ui})
QT4_ADD_RESOURCES(ftnoir-proto-fgfs-rcc ${ftnoir-proto-fgfs-rc})

file(GLOB ftnoir-proto-fsuipc-c "ftnoir_protocol_fsuipc/*.cpp")
file(GLOB ftnoir-proto-fsuipc-h "ftnoir_protocol_fsuipc/*.h")
QT4_WRAP_CPP(ftnoir-proto-fsuipc-moc ${ftnoir-proto-fsuipc-h})
file(GLOB ftnoir-proto-fsuipc-ui "ftnoir_protocol_fsuipc/*.ui")
file(GLOB ftnoir-proto-fsuipc-rc "ftnoir_protocol_fsuipc/*.qrc")
QT4_WRAP_UI(ftnoir-proto-fsuipc-uih ${ftnoir-proto-fsuipc-ui})
QT4_ADD_RESOURCES(ftnoir-proto-fsuipc-rcc ${ftnoir-proto-fsuipc-rc})

file(GLOB ftnoir-proto-freetrack-c "ftnoir_protocol_ft/*.cpp")
file(GLOB ftnoir-proto-freetrack-h "ftnoir_protocol_ft/*.h")
QT4_WRAP_CPP(ftnoir-proto-freetrack-moc ${ftnoir-proto-freetrack-h})
file(GLOB ftnoir-proto-freetrack-ui "ftnoir_protocol_ft/*.ui")
file(GLOB ftnoir-proto-freetrack-rc "ftnoir_protocol_ft/*.qrc")
QT4_WRAP_UI(ftnoir-proto-freetrack-uih ${ftnoir-proto-freetrack-ui})
QT4_ADD_RESOURCES(ftnoir-proto-freetrack-rcc ${ftnoir-proto-freetrack-rc})

file(GLOB ftnoir-proto-tir-c "ftnoir_protocol_ftir/*.cpp")
file(GLOB ftnoir-proto-tir-h "ftnoir_protocol_ftir/*.h")
QT4_WRAP_CPP(ftnoir-proto-tir-moc ${ftnoir-proto-tir-h})
file(GLOB ftnoir-proto-tir-ui "ftnoir_protocol_ftir/*.ui")
file(GLOB ftnoir-proto-tir-rc "ftnoir_protocol_ftir/*.qrc")
QT4_WRAP_UI(ftnoir-proto-tir-uih ${ftnoir-proto-tir-ui})
QT4_ADD_RESOURCES(ftnoir-proto-tir-rcc ${ftnoir-proto-tir-rc})

file(GLOB ftnoir-proto-udp-c "ftnoir_protocol_ftn/*.cpp")
file(GLOB ftnoir-proto-udp-h "ftnoir_protocol_ftn/*.h")
QT4_WRAP_CPP(ftnoir-proto-udp-moc ${ftnoir-proto-udp-h})
file(GLOB ftnoir-proto-udp-ui "ftnoir_protocol_ftn/*.ui")
file(GLOB ftnoir-proto-udp-rc "ftnoir_protocol_ftn/*.qrc")
QT4_WRAP_UI(ftnoir-proto-udp-uih ${ftnoir-proto-udp-ui})
QT4_ADD_RESOURCES(ftnoir-proto-udp-rcc ${ftnoir-proto-udp-rc})

file(GLOB ftnoir-proto-win32-mouse-c "ftnoir_protocol_mouse/*.cpp")
file(GLOB ftnoir-proto-win32-mouse-h "ftnoir_protocol_mouse/*.h")
QT4_WRAP_CPP(ftnoir-proto-win32-mouse-moc ${ftnoir-proto-win32-mouse-h})
file(GLOB ftnoir-proto-win32-mouse-ui "ftnoir_protocol_mouse/*.ui")
file(GLOB ftnoir-proto-win32-mouse-rc "ftnoir_protocol_mouse/*.qrc")
QT4_WRAP_UI(ftnoir-proto-win32-mouse-uih ${ftnoir-proto-win32-mouse-ui})
QT4_ADD_RESOURCES(ftnoir-proto-win32-mouse-rcc ${ftnoir-proto-win32-mouse-rc})

# TODO manifest problems :(
file(GLOB ftnoir-proto-simconnect-c "ftnoir_protocol_sc/*.cpp")
file(GLOB ftnoir-proto-simconnect-h "ftnoir_protocol_sc/*.h")
QT4_WRAP_CPP(ftnoir-proto-simconnect-moc ${ftnoir-proto-simconnect-h})
file(GLOB ftnoir-proto-simconnect-ui "ftnoir_protocol_sc/*.ui")
file(GLOB ftnoir-proto-simconnect-rc "ftnoir_protocol_sc/*.qrc")
QT4_WRAP_UI(ftnoir-proto-simconnect-uih ${ftnoir-proto-simconnect-ui})
QT4_ADD_RESOURCES(ftnoir-proto-simconnect-rcc ${ftnoir-proto-simconnect-rc})

# trackers

file(GLOB ftnoir-tracker-ht-c "ftnoir_tracker_ht/*.cpp")
file(GLOB ftnoir-tracker-ht-h "ftnoir_tracker_ht/*.h")
QT4_WRAP_CPP(ftnoir-tracker-ht-moc ${ftnoir-tracker-ht-h})
file(GLOB ftnoir-tracker-ht-ui "ftnoir_tracker_ht/*.ui")
file(GLOB ftnoir-tracker-ht-rc "ftnoir_tracker_ht/*.qrc")
QT4_WRAP_UI(ftnoir-tracker-ht-uih ${ftnoir-tracker-ht-ui})
QT4_ADD_RESOURCES(ftnoir-tracker-ht-rcc ${ftnoir-tracker-ht-rc})

file(GLOB ftnoir-tracker-pt-c "ftnoir_tracker_pt/*.cpp")
file(GLOB ftnoir-tracker-pt-h "ftnoir_tracker_pt/*.h")
QT4_WRAP_CPP(ftnoir-tracker-pt-moc ${ftnoir-tracker-pt-h})
file(GLOB ftnoir-tracker-pt-ui "ftnoir_tracker_pt/*.ui")
file(GLOB ftnoir-tracker-pt-rc "ftnoir_tracker_pt/*.qrc")
QT4_WRAP_UI(ftnoir-tracker-pt-uih ${ftnoir-tracker-pt-ui})
QT4_ADD_RESOURCES(ftnoir-tracker-pt-rcc ${ftnoir-tracker-pt-rc})

file(GLOB ftnoir-tracker-faceapi-c "ftnoir_tracker_sm/*.cpp")
file(GLOB ftnoir-tracker-faceapi-h "ftnoir_tracker_sm/*.h")
QT4_WRAP_CPP(ftnoir-tracker-faceapi-moc ${ftnoir-tracker-faceapi-h})
file(GLOB ftnoir-tracker-faceapi-ui "ftnoir_tracker_sm/*.ui")
file(GLOB ftnoir-tracker-faceapi-rc "ftnoir_tracker_sm/*.qrc")
QT4_WRAP_UI(ftnoir-tracker-faceapi-uih ${ftnoir-tracker-faceapi-ui})
QT4_ADD_RESOURCES(ftnoir-tracker-faceapi-rcc ${ftnoir-tracker-faceapi-rc})

file(GLOB ftnoir-tracker-udp-c "ftnoir_tracker_udp/*.cpp")
file(GLOB ftnoir-tracker-udp-h "ftnoir_tracker_udp/*.h")
QT4_WRAP_CPP(ftnoir-tracker-udp-moc ${ftnoir-tracker-udp-h})
file(GLOB ftnoir-tracker-udp-ui "ftnoir_tracker_udp/*.ui")
file(GLOB ftnoir-tracker-udp-rc "ftnoir_tracker_udp/*.qrc")
QT4_WRAP_UI(ftnoir-tracker-udp-uih ${ftnoir-tracker-udp-ui})
QT4_ADD_RESOURCES(ftnoir-tracker-udp-rcc ${ftnoir-tracker-udp-rc})

file(GLOB ftnoir-faceapi-wrapper-c "faceapi/*.cpp" "faceapi/*.rc")

# compat lib for POSIX/win32

file(GLOB ftnoir-compat-c "compat/*.cpp")

# some boilerplate

if(QT_USE_FILE)
	INCLUDE(${QT_USE_FILE})
endif()

# building modules

add_library(ftnoir-compat SHARED ${ftnoir-compat-c})
if(NOT WIN32)
    target_link_libraries(ftnoir-compat rt)
endif()

add_library(ftnoir-pose-widget SHARED ${ftnoir-pose-widget-c} ${ftnoir-pose-widget-moc} ${ftnoir-pose-widget-rcc})
target_link_libraries(ftnoir-pose-widget ${QT_LIBRARIES} ${OPENGL_gl_LIBRARY})
add_library(ftnoir-spline-widget SHARED ${ftnoir-spline-widget-c} ${ftnoir-spline-widget-moc})
target_link_libraries(ftnoir-spline-widget ${QT_LIBRARIES})

SET_TARGET_PROPERTIES(ftnoir-spline-widget PROPERTIES COMPILE_FLAGS "-DFTNOIR_FILTER_BASE_EXPORT -DQT_PLUGIN -DQDESIGNER_EXPORT_WIDGETS")

add_library(ftnoir-filter-accela SHARED ${ftnoir-filter-accela-c} ${ftnoir-filter-accela-moc} ${ftnoir-filter-accela-uih} ${ftnoir-filter-accela-rcc})
target_link_libraries(ftnoir-filter-accela ${QT_LIBRARIES} ftnoir-spline-widget)

SET_TARGET_PROPERTIES(ftnoir-filter-accela PROPERTIES COMPILE_FLAGS "-DFTNOIR_FILTER_BASE_LIB")
SET_TARGET_PROPERTIES(ftnoir-filter-accela
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")

add_library(ftnoir-filter-ewma SHARED ${ftnoir-filter-ewma-c} ${ftnoir-filter-ewma-moc} ${ftnoir-filter-ewma-uih} ${ftnoir-filter-ewma-rcc})
target_link_libraries(ftnoir-filter-ewma ${QT_LIBRARIES})

SET_TARGET_PROPERTIES(ftnoir-filter-ewma PROPERTIES COMPILE_FLAGS "-DFTNOIR_FILTER_BASE_LIB")
SET_TARGET_PROPERTIES(ftnoir-filter-ewma
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")

add_library(ftnoir-proto-fgfs SHARED ${ftnoir-proto-fgfs-c} ${ftnoir-proto-fgfs-moc} ${ftnoir-proto-fgfs-uih} ${ftnoir-proto-fgfs-rcc})
target_link_libraries(ftnoir-proto-fgfs ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(ftnoir-proto-fgfs
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")

if(WIN32)
    #add_library(ftnoir-proto-fsuipc SHARED ${ftnoir-proto-fsuipc-c} ${ftnoir-proto-fsuipc-moc} ${ftnoir-proto-fsuipc-uih} ${ftnoir-proto-fsuipc-rcc})
    #target_link_libraries(ftnoir-proto-fsuipc ${QT_LIBRARIES})
    #SET_TARGET_PROPERTIES(ftnoir-proto-fsuipc PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")

    if(SDK_SIMCONNECT)
        add_library(ftnoir-proto-simconnect SHARED ${ftnoir-proto-simconnect-c} ${ftnoir-proto-simconnect-moc} ${ftnoir-proto-simconnect-uih} ${ftnoir-proto-simconnect-rcc})
        target_link_libraries(ftnoir-proto-simconnect ${QT_LIBRARIES})
        SET_TARGET_PROPERTIES(ftnoir-proto-simconnect PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")
        include_directories("${SDK_SIMCONNECT}/inc")
        target_link_libraries(ftnoir-proto-simconnect "${SDK_SIMCONNECT}/lib/simconnect.lib")
    endif()

    add_library(ftnoir-proto-freetrack SHARED ${ftnoir-proto-freetrack-c} ${ftnoir-proto-freetrack-moc} ${ftnoir-proto-freetrack-uih} ${ftnoir-proto-freetrack-rcc})
    target_link_libraries(ftnoir-proto-freetrack ${QT_LIBRARIES})

    add_library(ftnoir-proto-tir SHARED ${ftnoir-proto-tir-c} ${ftnoir-proto-tir-moc} ${ftnoir-proto-tir-uih} ${ftnoir-proto-tir-rcc})
    target_link_libraries(ftnoir-proto-tir ${QT_LIBRARIES})
    SET_TARGET_PROPERTIES(ftnoir-proto-tir PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")
    add_library(ftnoir-proto-win32-mouse SHARED ${ftnoir-proto-win32-mouse-c} ${ftnoir-proto-win32-mouse-moc} ${ftnoir-proto-win32-mouse-uih} ${ftnoir-proto-win32-mouse-rcc})
    target_link_libraries(ftnoir-proto-win32-mouse ${QT_LIBRARIES})
    SET_TARGET_PROPERTIES(ftnoir-proto-win32-mouse PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")
    SET_TARGET_PROPERTIES(ftnoir-proto-freetrack PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")
endif()

add_library(ftnoir-proto-udp SHARED ${ftnoir-proto-udp-c} ${ftnoir-proto-udp-moc} ${ftnoir-proto-udp-uih} ${ftnoir-proto-udp-rcc})
target_link_libraries(ftnoir-proto-udp ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(ftnoir-proto-udp
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
SET_TARGET_PROPERTIES(ftnoir-proto-udp PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")

SET_TARGET_PROPERTIES(ftnoir-proto-fgfs PROPERTIES COMPILE_FLAGS "-DFTNOIR_PROTOCOL_BASE_LIB")
SET_TARGET_PROPERTIES(ftnoir-proto-fgfs
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")

add_library(ftnoir-tracker-ht SHARED ${ftnoir-tracker-ht-c} ${ftnoir-tracker-ht-moc} ${ftnoir-tracker-ht-uih} ${ftnoir-tracker-ht-rcc})
target_link_libraries(ftnoir-tracker-ht ${QT_LIBRARIES} ${OPENGL_gl_LIBRARY} ftnoir-compat)
SET_TARGET_PROPERTIES(ftnoir-tracker-ht
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")

if(OpenCV_FOUND)
    add_library(ftnoir-tracker-pt SHARED ${ftnoir-tracker-pt-c} ${ftnoir-tracker-pt-moc} ${ftnoir-tracker-pt-uih} ${ftnoir-tracker-pt-rcc})
    target_link_libraries(ftnoir-tracker-pt ${QT_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_gl_LIBRARY})
    SET_TARGET_PROPERTIES(ftnoir-tracker-pt PROPERTIES COMPILE_FLAGS "-DFTNOIR_TRACKER_BASE_LIB")
    SET_TARGET_PROPERTIES(ftnoir-tracker-pt
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
endif()

if(WIN32 AND SDK_SM_FACEAPI_PATH AND SDK_MSVC80_PATH)
    add_library(ftnoir-tracker-faceapi SHARED ${ftnoir-tracker-faceapi-c} ${ftnoir-tracker-faceapi-moc} ${ftnoir-tracker-faceapi-uih} ${ftnoir-tracker-faceapi-rcc})
    target_link_libraries(ftnoir-tracker-faceapi ${QT_LIBRARIES})
    SET_TARGET_PROPERTIES(ftnoir-tracker-faceapi PROPERTIES COMPILE_FLAGS "-DFTNOIR_TRACKER_BASE_LIB")
    include_directories("${SDK_SM_FACEAPI_PATH}/include")
    add_executable(ftnoir-faceapi-wrapper ${ftnoir-faceapi-wrapper-c})
    target_link_libraries(ftnoir-faceapi-wrapper "${SDK_SM_FACEAPI_PATH}/bin/smft32.lib" "${SDK_MSVC80_PATH}/lib/msvcrt.lib")
    SET_TARGET_PROPERTIES(ftnoir-faceapi-wrapper PROPERTIES COMPILE_FLAGS "-nostdlib")
    SET_TARGET_PROPERTIES(ftnoir-faceapi-wrapper PROPERTIES LINK_FLAGS "-Wl,-nostdlib")
endif()

add_library(ftnoir-tracker-udp SHARED ${ftnoir-tracker-udp-c} ${ftnoir-tracker-udp-moc} ${ftnoir-tracker-udp-uih} ${ftnoir-tracker-udp-rcc})
target_link_libraries(ftnoir-tracker-udp ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(ftnoir-tracker-udp
                  PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")

SET_TARGET_PROPERTIES(ftnoir-tracker-ht PROPERTIES COMPILE_FLAGS "-DFTNOIR_TRACKER_BASE_LIB")
SET_TARGET_PROPERTIES(ftnoir-tracker-udp PROPERTIES COMPILE_FLAGS "-DFTNOIR_TRACKER_BASE_LIB")

add_executable(ftnoir ${ftnoir-bin-c} ${ftnoir-bin-moc} ${ftnoir-bin-uih} ${ftnoir-bin-rcc})
target_link_libraries(ftnoir ftnoir-pose-widget ftnoir-spline-widget ${QT_LIBRARIES} ${QXT_QXTCORE_LIB_RELEASE} ${QXT_QXTGUI_LIB_RELEASE})
if(NOT WIN32)
    target_link_libraries(ftnoir dl)
endif()

if(WIN32)
    SET_TARGET_PROPERTIES(ftnoir PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows")
endif()

# make install

install(TARGETS
        ftnoir-compat
        ftnoir-pose-widget
        ftnoir-spline-widget
        ftnoir-filter-accela
        ftnoir-filter-ewma
        ftnoir-proto-fgfs
        ftnoir-proto-udp
        ftnoir-tracker-ht
		ftnoir-tracker-udp
        ftnoir
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .
        NAMELINK_SKIP
)

if(OpenCV_FOUND)
    install(TARGETS
            ftnoir-tracker-pt
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP
    )
endif()

if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION .)
    install(FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll" DESTINATION .)
    if(SDK_SIMCONNECT)
    install(TARGETS ftnoir-proto-simconnect
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP)
    endif()
    install(FILES
            "${SDK_MINGW_PATH}/bin/libgcc_s_dw2-1.dll"
            "${SDK_MINGW_PATH}/bin/libstdc++-6.dll"
            DESTINATION .
    )
    if(OpenCV_FOUND)
        get_filename_component(ftnoir-opencv-dir "${OpenCV_LIB_DIR_OPT}" PATH)
        file(GLOB ftnoir-opencv-files
            "${ftnoir-opencv-dir}/bin/libopencv_calib3d*.dll"
            "${ftnoir-opencv-dir}/bin/libopencv_core*.dll"
            "${ftnoir-opencv-dir}/bin/libopencv_highgui*.dll"
            "${ftnoir-opencv-dir}/bin/libopencv_imgproc*.dll"
            "${ftnoir-opencv-dir}/bin/libopencv_flann*.dll"
            "${ftnoir-opencv-dir}/bin/libopencv_features2d*.dll"
        )
        install(FILES ${ftnoir-opencv-files} DESTINATION .)
    endif()
    install(FILES "${QXT_QXTCORE_LIB_RELEASE}" "${QXT_QXTGUI_LIB_RELEASE}" DESTINATION .)
    get_filename_component(ftnoir-qt-bin "${QT_QMAKE_EXECUTABLE}" PATH)
    install(FILES
            "${ftnoir-qt-bin}/QtCore4.dll"
            "${ftnoir-qt-bin}/QtGui4.dll"
            "${ftnoir-qt-bin}/QtNetwork4.dll"
            "${ftnoir-qt-bin}/QtOpenGL4.dll"
            "${ftnoir-qt-bin}/QtXml4.dll"
            DESTINATION .
    )
    install(TARGETS
            ftnoir-proto-freetrack
            ftnoir-proto-tir
            ftnoir-proto-win32-mouse
            #ftnoir-proto-fsuipc
            #ftnoir-proto-simconnect
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP
    )
    if(WIN32)
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tracker-ht" DESTINATION .)
    endif()
    if(WIN32 AND SDK_SM_FACEAPI_PATH AND SDK_MSVC80_PATH)
        install(TARGETS ftnoir-tracker-faceapi
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .
                NAMELINK_SKIP
        )
        install(TARGETS ftnoir-faceapi-wrapper
                RUNTIME DESTINATION faceapi
                LIBRARY DESTINATION faceapi
                NAMELINK_SKIP
        )
        install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION faceapi)
        install(FILES
                "${SDK_SM_FACEAPI_PATH}/bin/computation6.0.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/foundation6.0.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/image6.0.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/libpng13.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/pgrflycapturegui.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/QtCore0.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/qt-mt334.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/QtXml4.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/smft32.dll"
                "${SDK_SM_FACEAPI_PATH}/bin/zlib1.dll"
                "${SDK_SM_FACEAPI_PATH}/doc/RELEASE NOTES.txt"
                DESTINATION faceapi
        )
        install(DIRECTORY
                "${SDK_SM_FACEAPI_PATH}/bin/cal"
                "${SDK_SM_FACEAPI_PATH}/bin/resources"
                DESTINATION faceapi
        )
    endif()
endif()

