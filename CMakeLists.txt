cmake_minimum_required(VERSION 3.8)
project(lift_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Generate interfaces FIRST
rosidl_generate_interfaces(${PROJECT_NAME}
  "action/Command.action"
  DEPENDENCIES std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

# Get the typesupport target
if(TARGET ${PROJECT_NAME})
  rosidl_get_typesupport_target(cpp_typesupport_target
    ${PROJECT_NAME} rosidl_typesupport_cpp)
endif()

# Action server as shared library
add_library(action_server SHARED
  src/lift_state.cpp
)
target_include_directories(action_server PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_definitions(action_server PRIVATE "LIFT_CONTROL_BUILDING_DLL")
ament_target_dependencies(action_server
  rclcpp
  rclcpp_action
  rclcpp_components
  std_msgs)

if(TARGET ${PROJECT_NAME})
  target_link_libraries(action_server "${cpp_typesupport_target}")
endif()

rclcpp_components_register_nodes(action_server "Lift_State")

install(TARGETS action_server
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# lift state executable
add_executable(lift_state src/lift_state.cpp)
ament_target_dependencies(lift_state 
  rclcpp 
  std_msgs 
  rclcpp_action 
  rclcpp_components)

if(TARGET ${PROJECT_NAME})
  target_link_libraries(lift_state "${cpp_typesupport_target}")
endif()

# lift command executable
add_executable(lift_command src/lift_command.cpp)
ament_target_dependencies(lift_command 
  rclcpp 
  std_msgs 
  rclcpp_action)

if(TARGET ${PROJECT_NAME})
  target_link_libraries(lift_command "${cpp_typesupport_target}")
endif()

install(TARGETS
  lift_state
  lift_command
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
