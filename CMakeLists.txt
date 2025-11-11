cmake_minimum_required(VERSION 3.8)
project(lift_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# lift state compilation instructions
add_executable(lift_state src/lift_state.cpp)
ament_target_dependencies(lift_state rclcpp std_msgs)
# lift command compilation instructions
add_executable(lift_command src/lift_command.cpp)
ament_target_dependencies(lift_command rclcpp std_msgs)

install(TARGETS
  lift_state
  lift_command
  DESTINATION lib/${PROJECT_NAME})

find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "action/Command.action"
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
