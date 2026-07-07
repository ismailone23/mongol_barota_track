#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "std_msgs/msg/string.hpp"
#include "rscp.pb.h"

#include <string>

class StageNode : public rclcpp::Node
{
public:
    StageNode() : Node("stage_node")
    {
        stage_sub_ =
            create_subscription<std_msgs::msg::ByteMultiArray>(
                "/Stage",
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
        const std_msgs::msg::ByteMultiArray::SharedPtr msg)
    {
        std::string command(msg->data.begin(), msg->data.end());

        if (command.rfind("SetStage(", 0) != 0)
        {
            RCLCPP_WARN(get_logger(), "Invalid SetStage command");
            return;
        }

        try
        {
            const auto start = command.find('(');
            const auto end = command.find(')');

            uint32_t stage = std::stoul(
                command.substr(start + 1, end - start - 1));

            current_stage_ = stage;

            RCLCPP_INFO(
                get_logger(),
                "Stage set to %u",
                current_stage_);

            send_ack();
        }
        catch (...)
        {
            RCLCPP_ERROR(get_logger(), "Failed to parse SetStage");
        }
    }

    void send_ack()
    {
        std_msgs::msg::String msg;
        msg.data = "ACK";

        ack_pub_->publish(msg);

        RCLCPP_INFO(get_logger(), "Stage ACK sent");
    }

    uint32_t current_stage_ = 0;

    rclcpp::Subscription<std_msgs::msg::ByteMultiArray>::SharedPtr
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