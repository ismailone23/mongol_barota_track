cmake_minimum_required(VERSION 3.8)
project(stage)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Protobuf REQUIRED)

# ===========================
# Stage Publisher
# ===========================
add_executable(stage_pub
  src/stage_pub.cpp
  src/rscp.pb.cc
)

ament_target_dependencies(stage_pub
  rclcpp
  std_msgs
)

target_include_directories(stage_pub PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(stage_pub
  protobuf::libprotobuf
)

# ===========================
# Host Module
# ===========================
add_executable(hm
  src/hm.cpp
  src/rscp.pb.cc
)

ament_target_dependencies(hm
  rclcpp
  std_msgs
)

target_include_directories(hm PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hm
  protobuf::libprotobuf
)

# ===========================
# Install
# ===========================
install(TARGETS
  stage_pub
  hm
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()