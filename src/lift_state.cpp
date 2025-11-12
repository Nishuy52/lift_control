#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "lift_control/action/command.hpp"

using namespace std::chrono_literals;

class Lift_State : public rclcpp::Node
{
  public:
    using Command = lift_control::action::Command;
    using GoalHandleCommand = rclcpp_action::ServerGoalHandle<Command>;
    explicit Lift_State(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("lift_state", options), count_(0)
    {
      //Initialize publisher
      publisher_ = this->create_publisher<std_msgs::msg::String>("lift_state", 10);
      
      //Initialize timer
      timer_ = this->create_wall_timer(
      500ms, std::bind(&Lift_State::timer_callback, this));
      
      //Initialize action server
      using namespace std::placeholders;
      this->action_server_ = rclcpp_action::create_server<Command>(
        this,
        "lift_action",
        std::bind(&Lift_State::handle_goal, this, _1, _2),
        std::bind(&Lift_State::handle_cancel, this, _1),
        std::bind(&Lift_State::handle_accepted, this, _1));
    }

  private:
    // Publisher Members
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    size_t count_;

    // Action Server Members
    rclcpp_action::Server<Command>::SharedPtr action_server_;

    // Publisher Callback
    void timer_callback()
    {
      auto message = std_msgs::msg::String();
      message.data = "Insert Data" + std::to_string(count_++);
      RCLCPP_INFO(this->get_logger(), "Lift State: '%s'", message.data.c_str());
      publisher_->publish(message);
    }

    // Action Server Callbacks
    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid,
                                             std::shared_ptr<const Command::Goal> goal)
    {
    RCLCPP_INFO(this->get_logger(), "Received command request: %d", goal->command);
    (void)uuid;
    
    // You can add validation logic here
    // For example, only accept certain command values
    if (goal->command < 0) {
      RCLCPP_WARN(this->get_logger(), "Rejecting negative command");
      return rclcpp_action::GoalResponse::REJECT;
    }
    
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }
  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleCommand> goal_handle)
  {
    RCLCPP_INFO(this->get_logger(), "Received request to cancel command");
    (void)goal_handle;
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandleCommand> goal_handle)
  {
    using namespace std::placeholders;
    // Spin up a new thread to avoid blocking the executor
    std::thread{std::bind(&Lift_State::execute, this, _1), goal_handle}.detach();
  }

  void execute(const std::shared_ptr<GoalHandleCommand> goal_handle)
  {
    RCLCPP_INFO(this->get_logger(), "Executing command");
    rclcpp::Rate loop_rate(2);  // 2 Hz for status updates
    
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<Command::Feedback>();
    auto result = std::make_shared<Command::Result>();
    
    // Simulate command execution with status updates
    int total_steps = 10;  // Example: 10 steps to complete command
    
    for (int i = 0; i <= total_steps && rclcpp::ok(); ++i) {
      // Check if there is a cancel request
      if (goal_handle->is_canceling()) {
        result->completion = (i * 100) / total_steps;  // Partial completion
        goal_handle->canceled(result);
        RCLCPP_INFO(this->get_logger(), "Command canceled at %d%% completion", 
                    result->completion);
        return;
      }
      
      // Update status (feedback)
      feedback->status = i;
      goal_handle->publish_feedback(feedback);
      RCLCPP_INFO(this->get_logger(), "Command progress - Status: %d/%d", i, total_steps);
      
      loop_rate.sleep();
    }
    
    // Command execution complete
    if (rclcpp::ok()) {
      result->completion = 100;  // 100% complete
      goal_handle->succeed(result);
      RCLCPP_INFO(this->get_logger(), "Command succeeded with completion: %d%%", 
                  result->completion);
    }
  }
};

RCLCPP_COMPONENTS_REGISTER_NODE(Lift_State)

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Lift_State>());
  rclcpp::shutdown();
  return 0;
}