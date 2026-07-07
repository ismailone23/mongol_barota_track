#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "rscp.pb.h"

#include <iostream>
#include <thread>

class TaskCompletedNode : public rclcpp::Node
{
public:
    TaskCompletedNode() : Node("task_completed_node")
    {
        task_pub_ =
            create_publisher<std_msgs::msg::ByteMultiArray>(
                "/TaskCompleted",
                10);

        input_thread_ =
            std::thread(
                &TaskCompletedNode::keyboard_loop,
                this);
    }

    ~TaskCompletedNode()
    {
        if (input_thread_.joinable())
            input_thread_.join();
    }

private:
    void keyboard_loop()
    {
        while (rclcpp::ok())
        {
            std::cout << "Type done to send TaskCompleted: ";

            std::string command;
            std::getline(std::cin, command);

            if (command == "done")
            {
                send_task_completed();
            }
        }
    }

    void send_task_completed()
    {
        rscp::ResponseEnvelope response;

        response.mutable_task_finished();

        std::string buffer;

        if (!response.SerializeToString(&buffer))
            return;

        std_msgs::msg::ByteMultiArray msg;
        msg.data.assign(buffer.begin(), buffer.end());

        task_pub_->publish(msg);

        RCLCPP_INFO(get_logger(), "TaskCompleted sent");
    }

    rclcpp::Publisher<std_msgs::msg::ByteMultiArray>::SharedPtr
        task_pub_;

    std::thread input_thread_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TaskCompletedNode>());
    rclcpp::shutdown();
}