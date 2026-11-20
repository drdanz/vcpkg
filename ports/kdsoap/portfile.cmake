vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/drdanz/KDSoap/releases/download/kdsoap-${VERSION}/kdsoap-${VERSION}.tar.gz"
    FILENAME "kdsoap-${VERSION}.tar.gz"
    SHA512 cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    PATCHES "fix-license-text.patch"
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" KDSoap_STATIC)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DKDSoap_QT6=ON
        -DKDSoap_STATIC=${KDSoap_STATIC}
        -DKDSoap_EXAMPLES=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME KDSoap-qt6 CONFIG_PATH lib/cmake/KDSoap-qt6)

vcpkg_copy_tools(TOOL_NAMES kdwsdl2cpp-qt6 AUTO_CLEAN)
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin")
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/doc")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(
    FILE_LIST
        "${SOURCE_PATH}/LICENSE.txt"
        "${SOURCE_PATH}/LICENSES/BSD-3-Clause.txt"
        "${SOURCE_PATH}/LICENSES/GPL-2.0-only.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-Microsoft.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-Novell.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-OASIS.txt"
        "${SOURCE_PATH}/LICENSES/LicenseRef-SportingExchange.txt"
        "${SOURCE_PATH}/LICENSES/MIT.txt"
        "${SOURCE_PATH}/LICENSES/W3C.txt"
)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
