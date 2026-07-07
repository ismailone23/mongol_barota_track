cmake_minimum_required(VERSION 3.8)
project(stage)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Protobuf REQUIRED)


set(RSCP_PROTO_SOURCE
  src/rscp.pb.cc
)

set(RSCP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

function(add_rscp_node TARGET_NAME SOURCE_FILE)

  add_executable(${TARGET_NAME}
    ${SOURCE_FILE}
    ${RSCP_PROTO_SOURCE}
  )

  ament_target_dependencies(${TARGET_NAME}
    rclcpp
    std_msgs
  )

  target_include_directories(${TARGET_NAME} PRIVATE
    ${Protobuf_INCLUDE_DIRS}
    ${RSCP_INCLUDE_DIR}
  )

  target_link_libraries(${TARGET_NAME}
    protobuf::libprotobuf
  )

endfunction()
add_rscp_node(cm
  src/cm.cpp
)

add_rscp_node(hm src/hm.cpp)

add_rscp_node(stage_node
  src/stage.cpp
)

add_rscp_node(arm_disarm
  src/arm_disarm.cpp
)

add_rscp_node(search_area
  src/search_area.cpp
)

# Rover Response/Event Nodes
add_rscp_node(gps_coordinate
  src/gps_coordinate.cpp
)

add_rscp_node(task_completed
  src/task_completed.cpp
)

# Install executables
install(TARGETS
  cm
  hm
  stage_node
  arm_disarm
  search_area
  gps_coordinate
  task_completed
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()