vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KDE/ktexttemplate
    REF "v${VERSION}"
    SHA512 bfc89e572ffbb5f906c11888882e2d2e99f206ce5397fa9805de35f316709d0be2b20e58dc691077201784583bd5632bfb7ce0e82f50e6c527a5bf73fdc6dd24
    HEAD_REF master
    #PATCHES fix-plugin-installation.patch
)
vcpkg_cmake_configure (
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME KF6TextTemplate CONFIG_PATH lib/cmake/KF6TextTemplate)
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(INSTALL "${SOURCE_PATH}/COPYING.LIB" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
