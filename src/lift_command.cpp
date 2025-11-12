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
    this->client_ptr_ = rclcpp_action::create_client<Command>(this, "lift_action");
    
    this->timer_ = this->create_wall_timer(
      std::chrono::milliseconds(500),
      std::bind(&Lift_Commander::send_goal, this));
  }

  void send_goal()
  {
    using namespace std::placeholders;
    this->timer_->cancel();

    if (!this->client_ptr_->wait_for_action_server()) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
      rclcpp::shutdown();
      return;
    }

    auto goal_msg = Command::Goal();
    goal_msg.command = 5;  // Example command value
    
    RCLCPP_INFO(this->get_logger(), "Sending goal with command: %d", goal_msg.command);

    auto send_goal_options = rclcpp_action::Client<Command>::SendGoalOptions();
    send_goal_options.goal_response_callback =
      std::bind(&Lift_Commander::goal_response_callback, this, _1);
    send_goal_options.feedback_callback =
      std::bind(&Lift_Commander::feedback_callback, this, _1, _2);
    send_goal_options.result_callback =
      std::bind(&Lift_Commander::result_callback, this, _1);
    
    this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
  }

private:
  // Subscriber members
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  
  // Action Client members
  rclcpp_action::Client<Command>::SharedPtr client_ptr_;
  rclcpp::TimerBase::SharedPtr timer_;

  // Subscriber function
  void topic_callback(const std_msgs::msg::String & msg) const
  {
    RCLCPP_INFO(this->get_logger(), "Reporting Lift State: '%s'", msg.data.c_str());
  }

  // Action Client functions
  void goal_response_callback(const GoalHandleCommand::SharedPtr & goal_handle)
  {
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
    } else {
      RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
    }
  }

  void feedback_callback(
    GoalHandleCommand::SharedPtr,
    const std::shared_ptr<const Command::Feedback> feedback)
  {
    RCLCPP_INFO(this->get_logger(), "Feedback - Status: %d", feedback->status);
  }

  void result_callback(const GoalHandleCommand::WrappedResult & result)
  {
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        RCLCPP_INFO(this->get_logger(), "Goal succeeded! Completion: %d%%", 
                    result.result->completion);
        break;
      case rclcpp_action::ResultCode::ABORTED:
        RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
        rclcpp::shutdown();
        return;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
        rclcpp::shutdown();
        return;
      default:
        RCLCPP_ERROR(this->get_logger(), "Unknown result code");
        rclcpp::shutdown();
        return;
    }
    rclcpp::shutdown();
  }
};

RCLCPP_COMPONENTS_REGISTER_NODE(Lift_Commander)

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Lift_Commander>(rclcpp::NodeOptions()));
  rclcpp::shutdown();
  return 0;
}