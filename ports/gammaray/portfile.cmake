vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KDAB/GammaRay
    REF 33d081aedfbf72304540c277fab27ff95c9e10b4
    SHA512 57441fa4dc51422130e70c17574b3fb315a58d9889cff8c222e7db558c6ae4f43beeb5631b6d846b7621ecd4b1be8b8b26c1519415fa18d8595100fa8cfd2c80
    HEAD_REF master
    PATCHES
        "fix-vcpkg-tools-relative-path.patch"
        "fix-license-text.patch"
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    INVERTED_FEATURES
        # "todo" GAMMARAY_PROBE_ONLY_BUILD
        # "foo" CMAKE_DISABLE_FIND_PACKAGE_KDSME-qt6

        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DAnimation
        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DCore
        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DExtras
        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DInput
        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DLogic
        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DQuick
        "3dinspector-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt63DRender
        "bluetooth-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6Bluetooth
        "core-probes" GAMMARAY_CLIENT_ONLY_BUILD
        "export-svg" CMAKE_DISABLE_FIND_PACKAGE_Qt6Svg
        "export-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6Designer
        "export-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6OpenGLWidgets
        "export-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6UiPlugin
        "positioning-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6Positioning
        "positioning-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6PositioningQuick
        "positioning-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6Location
        "qml-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6Qml
        "qml-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6QmlTools
        "quick-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6Quick
        "quick-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6QuickTools
        "quick-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6QuickWidgets
        "statemachine-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6Scxml
        "statemachine-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6ScxmlTools
        "statemachine-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6StateMachine
        "webinspector-ui" CMAKE_DISABLE_FIND_PACKAGE_Qt6WebEngineWidgets
        "wlcompositor-probe" CMAKE_DISABLE_FIND_PACKAGE_Qt6WaylandCompositor
        "wlcompositor-probe" CMAKE_DISABLE_FIND_PACKAGE_Wayland
    FEATURES
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DAnimation
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DCore
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DExtras
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DInput
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DLogic
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DQuick
        "3dinspector-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt63DRender
        "bluetooth-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Bluetooth
        "cli-injector" GAMMARAY_BUILD_CLI_INJECTOR
        "export-svg" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Svg
        "export-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Designer
        "export-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6OpenGLWidgets
        "export-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6UiPlugin
        "positioning-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Positioning
        "positioning-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Location
        "positioning-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6PositioningQuick
        "qml-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Qml
        "qml-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6QmlTools
        "quick-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Quick
        "quick-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6QuickTools
        "quick-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6QuickWidgets
        "statemachine-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6Scxml
        "statemachine-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6ScxmlTools
        "statemachine-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6StateMachine
        "ui-parts" GAMMARAY_BUILD_UI
        "webinspector-ui" CMAKE_REQUIRE_FIND_PACKAGE_Qt6WebEngineWidgets
        "wlcompositor-probe" CMAKE_REQUIRE_FIND_PACKAGE_Qt6WaylandCompositor
        "wlcompositor-probe" CMAKE_REQUIRE_FIND_PACKAGE_Wayland
    )

if(("widgets" IN_LIST FEATURES) OR ("ui-parts" IN_LIST FEATURES))
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6Widgets=ON)
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6WidgetsTools=ON)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6Widgets=OFF)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6WidgetsTools=OFF)
else()
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6Widgets=OFF)
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6WidgetsTools=OFF)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6Widgets=ON)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6WidgetsTools=ON)
endif()

if(("opengl-probe" IN_LIST FEATURES) OR ("export-ui" IN_LIST FEATURES))
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6OpenGL=ON)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6OpenGL=OFF)
else()
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6OpenGL=OFF)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6OpenGL=ON)
endif()

if(("3dinspector-probe" IN_LIST FEATURES) OR ("launcher" IN_LIST FEATURES))
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6Concurrent=ON)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6Concurrent=OFF)
else()
    list(APPEND FEATURE_OPTIONS -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6Concurrent=OFF)
    list(APPEND FEATURE_OPTIONS -DCMAKE_DISABLE_FIND_PACKAGE_Qt6Concurrent=ON)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DQT_VERSION_MAJOR=6
        -DGAMMARAY_BUILD_DOCS=OFF
        -DGAMMARAY_MULTI_BUILD=OFF
        -DGAMMARAY_INSTALL_QT_LAYOUT=OFF
        -DGAMMARAY_WITH_KDSME=OFF
        -DBUILD_TESTING=OFF
        -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6Gui=ON
        -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6GuiTools=ON
        -DCMAKE_REQUIRE_FIND_PACKAGE_Qt6Network=ON
        # Used by tests
        -DCMAKE_DISABLE_FIND_PACKAGE_Qt6Test=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_Qt6ShaderToolsTools=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_Qt6ShaderTools=ON
        # Unused
        -DCMAKE_DISABLE_FIND_PACKAGE_PkgConfig=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_QmlLint=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_Qt6QmlCompilerPlusPrivate=ON
        # Used for kjobtracker-probe
        -DCMAKE_DISABLE_FIND_PACKAGE_KF6CoreAddons=ON
        # Used for syntax highlighting
        -DCMAKE_DISABLE_FIND_PACKAGE_KF6SyntaxHighlighting=ON
        ${FEATURE_OPTIONS}
    MAYBE_UNUSED_VARIABLES
        CMAKE_DISABLE_FIND_PACKAGE_PkgConfig
        CMAKE_DISABLE_FIND_PACKAGE_Qt6ScxmlTools
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6ScxmlTools
        CMAKE_DISABLE_FIND_PACKAGE_Qt6ShaderTools
        CMAKE_DISABLE_FIND_PACKAGE_Qt6ShaderToolsTools
        CMAKE_DISABLE_FIND_PACKAGE_Qt6PositioningQuick
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6PositioningQuick
        CMAKE_DISABLE_FIND_PACKAGE_Qt6QmlCompilerPlusPrivate
        CMAKE_DISABLE_FIND_PACKAGE_Qt6QmlTools
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6QmlTools
        CMAKE_DISABLE_FIND_PACKAGE_Qt6QuickTools
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6QuickTools
        CMAKE_DISABLE_FIND_PACKAGE_Qt63Core
        CMAKE_DISABLE_FIND_PACKAGE_Qt63DCore
        CMAKE_REQUIRE_FIND_PACKAGE_Qt63DCore
        CMAKE_DISABLE_FIND_PACKAGE_Qt6WidgetsTools
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6WidgetsTools
        CMAKE_DISABLE_FIND_PACKAGE_Wayland
        CMAKE_DISABLE_FIND_PACKAGE_Qt6OpenGLWidgets
        CMAKE_DISABLE_FIND_PACKAGE_Qt6UiPlugin
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6OpenGLWidgets
        CMAKE_REQUIRE_FIND_PACKAGE_Qt6UiPlugin
        CMAKE_REQUIRE_FIND_PACKAGE_Wayland
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(PACKAGE_NAME GammaRay CONFIG_PATH lib/cmake/GammaRay)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/LICENSES")
file(REMOVE "${CURRENT_PACKAGES_DIR}/LICENSE.txt")
file(REMOVE "${CURRENT_PACKAGES_DIR}/README.md")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/zsh")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/LICENSES")
file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/LICENSE.txt")
file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/README.md")

vcpkg_install_copyright(
    FILE_LIST
        "${SOURCE_PATH}/LICENSE.txt"
        "${SOURCE_PATH}/LICENSES/BSD-2-Clause.txt"
        "${SOURCE_PATH}/LICENSES/BSD-3-Clause.txt"
        "${SOURCE_PATH}/LICENSES/GPL-2.0-only.txt"
        "${SOURCE_PATH}/LICENSES/GPL-2.0-or-later.txt"
        "${SOURCE_PATH}/LICENSES/GPL-3.0-only.txt"
        "${SOURCE_PATH}/LICENSES/GPL-3.0-or-later.txt"
        "${SOURCE_PATH}/LICENSES/LGPL-2.0-or-later.txt"
        "${SOURCE_PATH}/LICENSES/LGPL-2.1-only.txt"
        "${SOURCE_PATH}/LICENSES/LGPL-2.1-or-later.txt"
        "${SOURCE_PATH}/LICENSES/LGPL-3.0-only.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-CISST.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-Qt-Commercial.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-Qt-LGPL-EXCEPTION.txt"
        "${SOURCE_PATH}/LICENSES/MIT.txt"
)
file(
    COPY
        "${SOURCE_PATH}/README.md"
    DESTINATION
        "${CURRENT_PACKAGES_DIR}/share/${PORT}/"
)

vcpkg_copy_tools(
    TOOL_NAMES
        gammaray
    AUTO_CLEAN
)

if("ui" IN_LIST FEATURES)
    vcpkg_copy_tools(
        TOOL_NAMES
            gammaray-client
        AUTO_CLEAN
    )
endif()

if(VCPKG_TARGET_IS_WINDOWS AND "cli-injector" IN_LIST FEATURES)
    if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
        set(GAMMARAY_WININJECTOR_NAME "gammaray-wininjector-x86_64")
    else()
        set(GAMMARAY_WININJECTOR_NAME "gammaray-wininjector-i686")
    endif()
    vcpkg_copy_tools(
        TOOL_NAMES
            ${GAMMARAY_WININJECTOR_NAME}
        AUTO_CLEAN
    )
endif()

if("launcher" IN_LIST FEATURES)
    vcpkg_copy_tools(
        TOOL_NAMES
            gammaray-launcher
        AUTO_CLEAN
    )
endif()

file(COPY "${CURRENT_INSTALLED_DIR}/tools/Qt6/bin/qt.conf" DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}/")
vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/tools/${PORT}/qt.conf" "./../../../" "./../../")




#[[
NOTE:
  * webinspector-ui: currently qtwebengine cannot be installed on windows, see #42760
  * kjobtracker-probe: Requires KF6CoreAddons, currently not packaged
]]



    # "qtwayland",
    # "qtwebengine",
    # "kdstatemachineeditor",
    # "testlib",


#[[
    "default-features": [
    "3dinspector",
    "bluetooth",
    "cli-injector",
    "core-probes",
    "export-svg",
    "export-ui",
    "launcher",
    "location",
    "opengl-probe",
    "positioning",
    "qml",
    "quick",
    "statemachine",
    "ui",
    "widgets"
  ],


]]
