vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KDE/kio
    REF "v${VERSION}"
    SHA512 5b9d484ae499c924e695af41471dc00178067a9a69455b9f3e5436aa4dfb6effa803fe867e23ff8ae72a46e5006fdaf0c4531b2b0e5fbccd4c69eab12edc0af7
    HEAD_REF master
)

# Prevent KDEClangFormat from writing to source effectively blocking parallel configure
file(WRITE "${SOURCE_PATH}/.clang-format" "DisableFormat: true\nSortIncludes: false\n")

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        designerplugin BUILD_DESIGNERPLUGIN
        kf6wallet      VCPKG_LOCK_FIND_PACKAGE_KF6Wallet
    INVERTED_FEATURES
        translations   KF_SKIP_PO_PROCESSING
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DCMAKE_FIND_PACKAGE_TARGETS_GLOBAL=ON
        -DCMAKE_JOB_POOL_LINK=console # Serialize linking to avoid OOM
        -DBUILD_TESTING=OFF
        -DCMAKE_DISABLE_FIND_PACKAGE_ACL=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_KF6KDED=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_KF6DocTools=ON
        -DCMAKE_DISABLE_FIND_PACKAGE_SwitcherooControl=ON
        -DKDE_INSTALL_PLUGINDIR=plugins
        -DKDE_INSTALL_QTPLUGINDIR=plugins
        ${FEATURE_OPTIONS}
    MAYBE_UNUSED_VARIABLES
        CMAKE_JOB_POOL_LINK
        VCPKG_LOCK_FIND_PACKAGE_KF6Wallet
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/KF6KIO)
vcpkg_copy_pdbs()
vcpkg_copy_tools(
    TOOL_NAMES ktelnetservice6 ktrash6 kiod6 kioexec kioworker
    AUTO_CLEAN
)

file(APPEND "${CURRENT_PACKAGES_DIR}/tools/${PORT}/qt.conf" "Data = ../../share")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(GLOB LICENSE_FILES "${SOURCE_PATH}/LICENSES/*")
vcpkg_install_copyright(FILE_LIST ${LICENSE_FILES})
