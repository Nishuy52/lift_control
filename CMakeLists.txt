cmake_minimum_required(VERSION 3.8)
project(lift_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_library(action_server SHARED
  src/lift_state.cpp
)
target_include_directories(action_server PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_definitions(action_server PRIVATE "LIFT_CONTROL_BUILDING_DLL")
ament_target_dependencies(action_server
  "action_tutorials_interfaces"
  "rclcpp"
  "rclcpp_action"
  "rclcpp_components")
rclcpp_components_register_nodes(action_server "lift_control::Lift_State")
install(TARGETS action_server
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
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
