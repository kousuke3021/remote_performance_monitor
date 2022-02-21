cmake_minimum_required(VERSION 3.8)

add_definitions(-D_UNICODE -DUNICODE)

add_executable(${TARGET_NAME} WIN32 
 GUI.cpp
 CustomButton.cpp
 General_Info.cpp
 GPU_Info.cpp
 ssh.cpp
 util.cpp
 graph.cpp
 resource.rc
 icon.ico
)

# C++11
target_compile_features(${TARGET_NAME}
  PRIVATE cxx_std_14
)
target_compile_definitions(${TARGET_NAME}
  PRIVATE WITH_SOMELIB
)
find_package(OpenSSL REQUIRED)
find_package(Libssh2 REQUIRED CONFIG)


target_include_directories(${TARGET_NAME}
    PRIVATE
      ${OPENSSL_INCLUDE_DIR}
	${libssh2_INCLUDE_DIRS}
)
target_link_libraries(${TARGET_NAME}
    PRIVATE 
	${OPENSSL_CRYPTO_LIBRARY}
    	Libssh2::libssh2
)

target_sources(${TARGET_NAME} PRIVATE ${TARGET_NAME}.manifest )