cmake_minimum_required(VERSION 3.16)
project(NJU-PETs++ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)   # 自动处理 Q_OBJECT 宏
set(CMAKE_AUTORCC ON)   # 自动处理 .qrc 资源文件
set(CMAKE_AUTOUIC ON)   # 自动处理 .ui 文件

# ========================
# Find Qt Sections
# ========================
find_package(Qt6 REQUIRED COMPONENTS
        Widgets     # QWidget、QPainter、QMenu 等
        Sql         # QSqlDatabase、QSqlQuery 等
        Multimedia  # 音效播放（提醒音）
        Network     # QNetworkAccessManager（NLPService）
)

# ========================
# Get Srcs
# ========================
file(GLOB_RECURSE APP_SRCS
        "app/*.cpp"
        "app/*.h"
)

file(GLOB_RECURSE PRESENTATION_SRCS
        "presentation/*.cpp"
        "presentation/*.h"
)

file(GLOB_RECURSE BUSINESS_SRCS
        "business/*.cpp"
        "business/*.h"
)

file(GLOB_RECURSE DATA_SRCS
        "data/*.cpp"
        "data/*.h"
)

# ========================
# Create exe
# ========================
qt_add_executable(NJU-PETs++
        ${APP_SRCS}
        ${PRESENTATION_SRCS}
        ${BUSINESS_SRCS}
        ${DATA_SRCS}
        resources/resources.qrc
)


# 把项目根目录加进去，各层 include 时写完整相对路径
# 例如：#include "business/ScheduleService.h"
target_include_directories(NJU-PETs++ PRIVATE
        ${CMAKE_SOURCE_DIR}
)

# ========================
# Link Qt libraries
# ========================
target_link_libraries(NJU-PETs++ PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Multimedia
        Qt6::Network
)

# ========================
# macOS / Windows Settings
# ========================
if(APPLE)
    set_target_properties(NJU-PETs++ PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "DesktopPet"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourname.desktopPet"
    )
endif()

if(WIN32)
    set_target_properties(NJU-PETs++ PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

# ========================
# clang-format target
# ========================
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)

    file(GLOB_RECURSE ALL_SOURCE_FILES
            "app/*.cpp"          "app/*.h"
            "presentation/*.cpp" "presentation/*.h"
            "business/*.cpp"     "business/*.h"
            "data/*.cpp"         "data/*.h"
    )
    add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
            COMMENT "Running clang-format on all source files..."
    )
endif()