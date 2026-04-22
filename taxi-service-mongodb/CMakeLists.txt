cmake_minimum_required(VERSION 3.12...3.31)
project(taxi_api CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(
    userver
    COMPONENTS core mongo
    QUIET
)
if(NOT userver_FOUND)
    download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

# Common sources
include_directories(src)

# Create static library with main code
add_library(${PROJECT_NAME}_lib STATIC
    # Models
    src/models/user.cpp
    src/models/driver.cpp
    src/models/ride.cpp
    
    # Auth
    src/auth/jwt_manager.cpp
    src/auth/auth_middleware.cpp
    
    # Handlers
    src/handlers/register_user.cpp
    src/handlers/find_user_by_login.cpp
    src/handlers/search_users_by_name.cpp
    src/handlers/register_driver.cpp
    src/handlers/create_ride.cpp
    src/handlers/get_active_rides.cpp
    src/handlers/accept_ride.cpp
    src/handlers/get_user_rides.cpp
    src/handlers/complete_ride.cpp
    src/handlers/auth_login.cpp
    src/handlers/auth_register.cpp
    src/handlers/openapi_handler.cpp
    src/handlers/swagger_ui_handler.cpp
)

target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC userver::core
           userver::mongo
)

# The Service - executable file
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

# Unit Tests (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/unit_test.cpp)
    add_executable(${PROJECT_NAME}_unittest src/tests/unit_test.cpp)
    target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_lib userver::utest)
    add_google_tests(${PROJECT_NAME}_unittest)
endif()

# Functional testing
userver_testsuite_add_simple()

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)

install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/openapi.yaml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)