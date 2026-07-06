#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "rscp.pb.h"
#include <iomanip>
#include <sstream>

class Stage : public rclcpp::Node
{
public:
    Stage() : Node("rscp_stage_pub")
    {
        ack_publisher_ = this->create_publisher<std_msgs::msg::ByteMultiArray>("/Acknowledge", 10);
        stage_subscriber_ = this->create_subscription<std_msgs::msg::ByteMultiArray>("/Stage", 10,
                                                                                     std::bind(&Stage::receive_callback,
                                                                                               this,
                                                                                               std::placeholders::_1));
    }

private:
    bool stage_ack_ = false;

    rclcpp::Publisher<std_msgs::msg::ByteMultiArray>::SharedPtr ack_publisher_;
    rclcpp::Subscription<std_msgs::msg::ByteMultiArray>::SharedPtr stage_subscriber_;
    uint32_t current_stage_;
    bool arm_disarm = false;

    void receive_callback(const std_msgs::msg::ByteMultiArray::SharedPtr msg)
    {
        stage_ack_ = false;

        // Convert ByteMultiArray -> std::string
        std::string data(msg->data.begin(), msg->data.end());

        rscp::RequestEnvelope request;

        if (!request.ParseFromString(data))
        {
            RCLCPP_WARN(get_logger(), "Failed to parse RequestEnvelope");
            return;
        }

        // Check if this packet contains SetStage

        if (request.has_set_stage())
        {
            uint32_t stage = request.set_stage().value();

            RCLCPP_INFO(get_logger(), "Received SetStage(%u)", stage);

            // Example: store the stage
            current_stage_ = stage;

            // Send acknowledgement
            send_ack();
        }
        if (request.has_arm_disarm())
        {
            bool arm = request.arm_disarm().value();

            RCLCPP_INFO(get_logger(), "Received arm_disarm(%u)", arm);

            // Example: store the stage
            arm_disarm = arm;

            // Send acknowledgement
            send_ack();
        }

        if (request.has_search_area())
        {
        }
        if (request.has_navigate_to_gps())
        {
        }
        if (request.has_start_exploration())
        {
        }
    }
    void send_ack()
    {
        rscp::ResponseEnvelope response;

        response.mutable_acknowledge();

        std::string buffer;
        response.SerializeToString(&buffer);

        std_msgs::msg::ByteMultiArray msg;
        msg.data.assign(buffer.begin(), buffer.end());

        ack_publisher_->publish(msg);

        RCLCPP_INFO(get_logger(), "ACK sent");
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<Stage>();
    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}