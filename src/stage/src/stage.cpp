#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/u_int32.hpp"

#include <cstdint>

class StageNode : public rclcpp::Node
{
public:
    StageNode() : Node("stage_node")
    {
        stage_sub_ =
            create_subscription<std_msgs::msg::UInt32>(
                "/SetStage",
                10,
                std::bind(
                    &StageNode::callback,
                    this,
                    std::placeholders::_1));

        ack_pub_ =
            create_publisher<std_msgs::msg::String>(
                "/Acknowledge",
                10);
    }

private:
    void callback(
        const std_msgs::msg::UInt32::SharedPtr msg)
    {
        current_stage_ = msg->data;

        RCLCPP_INFO(
            get_logger(),
            "Stage set to %u",
            current_stage_);

        send_ack();
    }

    void send_ack()
    {
        std_msgs::msg::String msg;
        msg.data = "ACK";

        ack_pub_->publish(msg);

        RCLCPP_INFO(get_logger(), "Stage ACK sent");
    }

    uint32_t current_stage_ = 0;

    rclcpp::Subscription<std_msgs::msg::UInt32>::SharedPtr
        stage_sub_;

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr
        ack_pub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StageNode>());
    rclcpp::shutdown();
}
