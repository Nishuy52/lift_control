#include <functional>
#include <future>
#include <memory>
#include <string>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "lift_control/action/command.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

class Lift_Commander : public rclcpp::Node
{
public:
  using Command = lift_control::action::Command;
  using GoalHandleCommand = rclcpp_action::ClientGoalHandle<Command>;

  explicit Lift_Commander(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("lift_commander", options)
  {
    // Initialize subscriber
    subscription_ = this->create_subscription<std_msgs::msg::String>(
      "lift_state", 10, std::bind(&Lift_Commander::topic_callback, this, _1));
    
    // Initialize action client
    RCLCPP_INFO(this->get_logger(), "Lift Commander initialized");
    RCLCPP_INFO(this->get_logger(), "Waiting for action server...");
    
    if (!this->client_ptr_->wait_for_action_server(std::chrono::seconds(5))) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available!");
    } else {
      RCLCPP_INFO(this->get_logger(), "Action server connected!");
      print_menu();
    }
  }
  void send_command(int command)
  {
    if (goal_in_progress_) {
      RCLCPP_WARN(this->get_logger(), "A goal is already in progress. Wait or send STOP.");
      return;
    }

    auto goal_msg = Command::Goal();
    goal_msg.command = command;
    
    std::string command_name;
    switch(command) {
      case 0: command_name = "STOP"; break;
      case 1: command_name = "GO UP"; break;
      case 2: command_name = "GO DOWN"; break;
      default: command_name = "UNKNOWN"; break;
    }
    
    RCLCPP_INFO(this->get_logger(), "Sending command: %s (%d)", 
                command_name.c_str(), command);

    auto send_goal_options = rclcpp_action::Client<Command>::SendGoalOptions();
    send_goal_options.goal_response_callback =
      std::bind(&Lift_Commander::goal_response_callback, this, _1);
    send_goal_options.feedback_callback =
      std::bind(&Lift_Commander::feedback_callback, this, _1, _2);
    send_goal_options.result_callback =
      std::bind(&Lift_Commander::result_callback, this, _1);
    
    goal_in_progress_ = true;
    this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
  }

  void print_menu()
  {
    std::cout << "\n=================================\n";
    std::cout << "    LIFT CONTROL INTERFACE\n";
    std::cout << "=================================\n";
    std::cout << "Commands:\n";
    std::cout << "  0 - STOP (Emergency stop)\n";
    std::cout << "  1 - GO UP (Move to second level)\n";
    std::cout << "  2 - GO DOWN (Move to first level)\n";
    std::cout << "  q - Quit\n";
    std::cout << "=================================\n";
    std::cout << "Enter command: ";
    std::cout.flush();
  }

private:
  // Subscriber members
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  
  // Action Client members
  rclcpp_action::Client<Command>::SharedPtr client_ptr_;
  rclcpp::TimerBase::SharedPtr timer_;

  // Lift Status members
  bool goal_in_progress_;
  std::mutex goal_mutex_;


  // Subscriber function
  void topic_callback(const std_msgs::msg::String & msg) const
  {
    RCLCPP_INFO(this->get_logger(), "Current Lift State: '%s'", msg.data.c_str());
  }

  // Action Client functions
  void goal_response_callback(const GoalHandleCommand::SharedPtr & goal_handle)
  {
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Goal was REJECTED by server!");
      RCLCPP_ERROR(this->get_logger(), "Command not allowed in current lift state.");
      goal_in_progress_ = false;
      print_menu();
    } else {
      RCLCPP_INFO(this->get_logger(), "Goal ACCEPTED by server, executing...");
    }
  }

  void feedback_callback(
    GoalHandleCommand::SharedPtr,
    const std::shared_ptr<const Command::Feedback> feedback)
  {
    if (feedback->status == 0) {
      RCLCPP_INFO(this->get_logger(), "Status: MOVING...");
    } else {
      RCLCPP_INFO(this->get_logger(), "Status: REACHED target");
    }
  }

  void result_callback(const GoalHandleCommand::WrappedResult & result)
  {
    goal_in_progress_ = false;
    
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        if (result.result->completion == 1) {
          RCLCPP_INFO(this->get_logger(), "✓✓ Goal SUCCEEDED! Lift reached target level.");
        } else {
          RCLCPP_INFO(this->get_logger(), "✓ Goal completed. Lift stopped.");
        }
        break;
      case rclcpp_action::ResultCode::ABORTED:
        RCLCPP_ERROR(this->get_logger(), "❌ Goal was ABORTED");
        break;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_WARN(this->get_logger(), "⚠ Goal was CANCELED");
        break;
      default:
        RCLCPP_ERROR(this->get_logger(), "❌ Unknown result code");
        break;
    }
    print_menu();
  }
};

//RCLCPP_COMPONENTS_REGISTER_NODE(Lift_Commander)

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  
  auto node = std::make_shared<Lift_Commander>(rclcpp::NodeOptions());
  
  // Spin in a separate thread
  std::thread spin_thread([node]() {
    rclcpp::spin(node);
  });
  
  // CLI loop in main thread
  std::string input;
  while (rclcpp::ok()) {
    std::getline(std::cin, input);
    
    if (input.empty()) continue;
    
    if (input == "q" || input == "Q") {
      RCLCPP_INFO(node->get_logger(), "Shutting down...");
      rclcpp::shutdown();
      break;
    }
    
    try {
      int command = std::stoi(input);
      if (command >= 0 && command <= 2) {
        node->send_command(command);
      } else {
        std::cout << "Invalid command! Use 0 (STOP), 1 (UP), 2 (DOWN), or q (QUIT)\n";
        std::cout << "Enter command: ";
        std::cout.flush();
      }
    } catch (const std::exception& e) {
      std::cout << "Invalid input! Use 0, 1, 2, or q\n";
      std::cout << "Enter command: ";
      std::cout.flush();
    }
  }
  
  if (spin_thread.joinable()) {
    spin_thread.join();
  }
  
  return 0;
}