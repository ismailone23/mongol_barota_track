#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/string.hpp"

class ArmDisarmNode : public rclcpp::Node
{
public:
    ArmDisarmNode() : Node("arm_disarm_node")
    {
        command_sub_ =
            create_subscription<std_msgs::msg::Bool>(
                "/ArmDisarm",
                10,
                std::bind(
                    &ArmDisarmNode::callback,
                    this,
                    std::placeholders::_1));

        ack_pub_ =
            create_publisher<std_msgs::msg::String>(
                "/Acknowledge",
                10);
    }

private:
    void callback(
        const std_msgs::msg::Bool::SharedPtr msg)
    {
        armed_ = msg->data;

        RCLCPP_INFO(
            get_logger(),
            "Rover armed state: %s",
            armed_ ? "TRUE" : "FALSE");

        send_ack();
    }

    void send_ack()
    {
        std_msgs::msg::String msg;
        msg.data = "ACK";

        ack_pub_->publish(msg);

        RCLCPP_INFO(get_logger(), "ArmDisarm ACK sent");
    }

    bool armed_ = false;

    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr
        command_sub_;

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr
        ack_pub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ArmDisarmNode>());
    rclcpp::shutdown();
}
