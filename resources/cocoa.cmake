get_target_property(GMP_LIB GMP_STATIC IMPORTED_LOCATION)
set(GMP_LIB_ARG "--with-libgmp=${GMP_LIB}")

if(APPLE)
	set(SEDCMD sed -i ".bak")
else()
	set(SEDCMD sed -i)
endif()

ExternalProject_Add(
    CoCoALib-EP
	URL "http://cocoa.dima.unige.it/cocoalib/tgz/CoCoALib-${COCOA_VERSION}.tgz"
	URL_MD5 ${COCOA_TGZHASH}
	DOWNLOAD_NO_PROGRESS 1
	BUILD_IN_SOURCE YES
	PATCH_COMMAND sh ${CMAKE_SOURCE_DIR}/resources/cocoa/patch_auto_ptr.sh
	COMMAND patch <SOURCE_DIR>/src/AlgebraicCore/TmpFactorDir/DUPZfactor_combine.c ${CMAKE_SOURCE_DIR}/resources/cocoa/DUPZfactor_combine.c.patch
	CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --threadsafe-hack ${GMP_LIB_ARG}
	BUILD_COMMAND make library
	INSTALL_COMMAND ${CMAKE_COMMAND} -E touch <SOURCE_DIR>/examples/index.html
	COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
	COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/lib
	COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/CoCoA-${COCOA_VERSION}
	COMMAND ${CMAKE_MAKE_PROGRAM} install
	BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/resources/src/CoCoALib-EP/lib/libcocoa${STATIC_EXT}
)

ExternalProject_Get_Property(CoCoALib-EP INSTALL_DIR)

add_imported_library(COCOA SHARED "${INSTALL_DIR}/lib/libcocoa${STATIC_EXT}" "${INSTALL_DIR}/include")
add_imported_library(COCOA STATIC "${INSTALL_DIR}/lib/libcocoa${STATIC_EXT}" "${INSTALL_DIR}/include")

add_dependencies(CoCoALib-EP GMP_STATIC)
add_dependencies(COCOA_SHARED CoCoALib-EP GMPXX_SHARED)
add_dependencies(COCOA_STATIC CoCoALib-EP GMP_STATIC)
add_dependencies(resources COCOA_SHARED COCOA_STATIC)
