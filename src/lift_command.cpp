#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
using std::placeholders::_1;

class receive_Lift_State : public rclcpp::Node
{
  public:
    receive_Lift_State()
    : Node("recieve_lift_state")
    {
      subscription_ = this->create_subscription<std_msgs::msg::String>(
      "lift_state", 10, std::bind(&receive_Lift_State::topic_callback, this, _1));
    }

  private:
    void topic_callback(const std_msgs::msg::String & msg) const
    {
      RCLCPP_INFO(this->get_logger(), "Reporting Lift State: '%s'", msg.data.c_str());
    }
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<receive_Lift_State>());
  rclcpp::shutdown();
  return 0;
}