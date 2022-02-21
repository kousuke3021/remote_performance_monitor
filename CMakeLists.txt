cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME remote_performance_monitor)
# プロジェクト名を指定する
project(${TARGET_NAME})

# 次の行を追加するとMinSizeRelとRelWithDebInfoの選択肢を生成することが抑制できます。
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)

add_subdirectory(src)
