cmake_minimum_required(VERSION 3.5.1)
project(sgx_tcv)

######################## TARGET NAME ########################

set(TRUSTED_LIB        "sgx_tcv")
set(UNTRUSTED_LIB      "sgx_ucv")


######################## SGX SETTINGS ########################

set(SGX_SDK            /opt/intel/sgxsdk)
set(SGX_COMMON_CFLAGS  "-m64")
set(SGX_EDGER8R        ${SGX_SDK}/bin/x64/sgx_edger8r)
set(SGX_LIBRARY_PATH   ${SGX_SDK}/lib)


######################## COMPILE FLAGS ########################

set(SGX_TCV_FLAGS     "-DENABLE_SGX_SUPPORT -DOPENCV_NOSTL -UHAVE_OPENCL -DNO_GETENV -D__OPENCV_BUILD")

set(ENCLAVE_C_FLAGS   "${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
set(ENCLAVE_CXX_FLAGS "${ENCLAVE_C_FLAGS} -std=c++03 -nostdinc++ ${SGX_TCV_FLAGS}")


######################## EDL TRUSTED OBJECT ########################

set(EDL_NAME          "OpencvCore")
set(EDL_ABSPATH       "${CMAKE_CURRENT_SOURCE_DIR}/${EDL_NAME}.edl")
set(EDL_T_C           "${CMAKE_CURRENT_SOURCE_DIR}/trusted/${EDL_NAME}_t.c")
set(EDL_SEARCH_PATHS  "${SGX_SDK}/include")

add_custom_command(OUTPUT ${EDL_T_C}
                   COMMAND ${SGX_EDGER8R} --trusted ${EDL_ABSPATH} --search-path ${EDL_SEARCH_PATHS}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/trusted)

add_library(${PROJECT_NAME}-edlobj OBJECT ${EDL_T_C})

string(REPLACE " " ";" REPLACED_C_FLAGS ${ENCLAVE_C_FLAGS})
target_compile_options(${PROJECT_NAME}-edlobj PRIVATE ${REPLACED_C_FLAGS})

target_include_directories(${PROJECT_NAME}-edlobj PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/trusted
	${SGX_SDK}/include
	${SGX_SDK}/include/tlibc
	${SGX_SDK}/include/stlport)


######################## TRUSTED STATIC LIBRARY ########################

file(GLOB TRUSTED_SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/trusted/*.cpp")

add_library(${PROJECT_NAME} STATIC
	${TRUSTED_SRCS}
	$<TARGET_OBJECTS:${PROJECT_NAME}-edlobj>)

string(REPLACE " " ";" REPLACED_CXX_FLAGS ${ENCLAVE_CXX_FLAGS})
target_compile_options(${PROJECT_NAME} PRIVATE ${REPLACED_CXX_FLAGS})

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/trusted
	${SGX_SDK}/include
	${SGX_SDK}/include/tlibc
	${SGX_SDK}/include/stlport
	/usr/lib/gcc/x86_64-linux-gnu/${CMAKE_CXX_COMPILER_VERSION}/include)

target_link_libraries(${PROJECT_NAME} "${SGX_COMMON_CFLAGS} \
	-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
	-Wl,--whole-archive -lsgx_trts -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -lsgx_tkey_exchange -lsgx_tcrypto -lsgx_tservice -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
	-Wl,--defsym,__ImageBase=0")


######################## UNTRUSTED STATIC LIBRARY ########################

set(EDL_U_C           "${CMAKE_CURRENT_SOURCE_DIR}/untrusted/${EDL_NAME}_u.c")

add_custom_command(OUTPUT ${EDL_U_C}
                   COMMAND ${SGX_EDGER8R} --untrusted ${EDL_ABSPATH} --search-path ${EDL_SEARCH_PATHS}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/untrusted)

file(GLOB UNTRUSTED_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/untrusted/*.cpp")

add_library(${UNTRUSTED_LIB} STATIC
        ${UNTRUSTED_SRCS}
        ${EDL_U_C})

target_include_directories(${UNTRUSTED_LIB} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/untrusted
        ${SGX_SDK}/include)

target_link_libraries(${UNTRUSTED_LIB} -lopencv_core)
