vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KDE/kiconthemes
    REF "v${VERSION}"
    SHA512 c995371220695ec0c38c14b8399eca92b93101189515348596eec8e0640e688a36fc85f4ffe30ab65ebd23e5bf1911526b34921bd910bdab7fb574f857bb2f0e
    HEAD_REF master
)

# Prevent KDEClangFormat from writing to source effectively blocking parallel configure
file(WRITE "${SOURCE_PATH}/.clang-format" "DisableFormat: true\nSortIncludes: false\n")

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        designerplugin BUILD_DESIGNERPLUGIN
        kf6breezeicons USE_BreezeIcons
    INVERTED_FEATURES
        translations   KF_SKIP_PO_PROCESSING
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
        -DKDE_INSTALL_PLUGINDIR=plugins
        -DKDE_INSTALL_QTPLUGINDIR=plugins
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/KF6IconThemes)
vcpkg_copy_pdbs()

vcpkg_copy_tools(
    TOOL_NAMES kiconfinder6
    AUTO_CLEAN
)

if(VCPKG_TARGET_IS_OSX)
    vcpkg_copy_tools(TOOL_NAMES ksvg2icns AUTO_CLEAN)
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(GLOB LICENSE_FILES "${SOURCE_PATH}/LICENSES/*")
vcpkg_install_copyright(FILE_LIST ${LICENSE_FILES})
