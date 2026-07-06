cmake_minimum_required(VERSION 3.8)
project(rscpcoms)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Protobuf REQUIRED)

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()


add_executable(receiver
    src/receiver.cpp
    src/rscp.pb.cc
)

ament_target_dependencies(receiver
    rclcpp
    std_msgs
)

target_include_directories(receiver PRIVATE
    include
)

target_link_libraries(receiver
    ${Protobuf_LIBRARIES}
)

add_executable(sender
    src/sender.cpp
    src/rscp.pb.cc
)

ament_target_dependencies(sender
    rclcpp
    std_msgs
)

target_include_directories(sender PRIVATE
    include
)

target_link_libraries(sender
    ${Protobuf_LIBRARIES}
)

install(TARGETS
  receiver
  sender
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
