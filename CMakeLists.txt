# Top-level ESP-IDF project file, see
# https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/build-system.html
cmake_minimum_required(VERSION 3.22)
# Layer the git-ignored secrets file over the committed defaults, if present.
# https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/build-system.html#custom-sdkconfig-defaults
set(SDKCONFIG_DEFAULTS "sdkconfig.defaults")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.secrets")
    list(APPEND SDKCONFIG_DEFAULTS "sdkconfig.secrets")
endif()
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(c-coaster)
