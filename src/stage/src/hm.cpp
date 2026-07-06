#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "rscp.pb.h"

class HMNode : public rclcpp::Node
{
public:
    HMNode() : Node("host_module")
    {
        request_pub_ = this->create_publisher<std_msgs::msg::ByteMultiArray>("/Stage", 10);
        response_sub_ = this->create_subscription<std_msgs::msg::ByteMultiArray>("/Acknowledge",
                                                                                 10,
                                                                                 std::bind(&HMNode::response_callback,
                                                                                           this,
                                                                                           std::placeholders::_1));
        keyboard_thread_ = std::thread(&HMNode::keyboard_loop, this);
    }

    ~HMNode()
    {
        if (keyboard_thread_.joinable())
            keyboard_thread_.join();
    }

private:
    void keyboard_loop()
    {
        while (rclcpp::ok())
        {
            std::cout << "> ";

            std::string command;
            std::getline(std::cin, command);

            if (command.rfind("SetStage(", 0) == 0)
            {
                auto start = command.find('(');
                auto end = command.find(')');

                if (start != std::string::npos && end != std::string::npos)
                {
                    uint32_t stage = std::stoul(command.substr(start + 1, end - start - 1));
                    send_set_stage(stage);
                }
            }
            else if (command == "ArmDisarm(arm=True)")
            {
                send_arm_disarm(true);
            }
            else if (command == "ArmDisarm(arm=False)")
            {
                send_arm_disarm(false);
            }
            else if (command == "q" || command == "exit" || command == "exit()")
            {
                rclcpp::shutdown();
                return;
            }
            else
            {
                std::cout << "Unknown command\n";
            }
        }
    }

    void send_arm_disarm(bool arm)
    {
        rscp::RequestEnvelope request;

        request.mutable_arm_disarm()->set_value(arm);

        std::string buffer;
        request.SerializeToString(&buffer);

        std_msgs::msg::ByteMultiArray msg;
        msg.data.assign(buffer.begin(), buffer.end());

        request_pub_->publish(msg);
    }

    void send_set_stage(int stage_val)
    {
        rscp::RequestEnvelope request;

        request.mutable_set_stage()->set_value(stage_val);

        std::string buffer;
        request.SerializeToString(&buffer);

        std_msgs::msg::ByteMultiArray msg;
        msg.data.assign(buffer.begin(), buffer.end());

        request_pub_->publish(msg);
    }

    void response_callback(
        const std_msgs::msg::ByteMultiArray::SharedPtr msg)
    {
        rscp::ResponseEnvelope response;

        std::string data(msg->data.begin(), msg->data.end());

        if (response.ParseFromString(data))
        {
            if (response.has_acknowledge())
            {
                RCLCPP_INFO(get_logger(), "Stage acknowledged!");
            }
        }
    }

    rclcpp::Publisher<std_msgs::msg::ByteMultiArray>::SharedPtr request_pub_;

    rclcpp::Subscription<std_msgs::msg::ByteMultiArray>::SharedPtr response_sub_;

    std::thread keyboard_thread_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<HMNode>();
    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}