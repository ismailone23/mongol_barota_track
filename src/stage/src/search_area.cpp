#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "std_msgs/msg/string.hpp"

class SearchAreaNode : public rclcpp::Node
{
public:
    SearchAreaNode() : Node("search_area_node")
    {
        command_sub_ =
            create_subscription<std_msgs::msg::Float64MultiArray>(
                "/SearchArea",
                10,
                std::bind(
                    &SearchAreaNode::callback,
                    this,
                    std::placeholders::_1));

        ack_pub_ =
            create_publisher<std_msgs::msg::String>(
                "/Acknowledge",
                10);
    }

private:
    void callback(
        const std_msgs::msg::Float64MultiArray::SharedPtr msg)
    {
        if (msg->data.size() != 3)
        {
            RCLCPP_WARN(
                get_logger(),
                "Invalid SearchArea value count: expected 3, got %zu",
                msg->data.size());
            return;
        }

        latitude_ = msg->data[0];
        longitude_ = msg->data[1];
        radius_ = msg->data[2];

        RCLCPP_INFO(
            get_logger(),
            "SearchArea lat=%.6f lon=%.6f radius=%.2f",
            latitude_,
            longitude_,
            radius_);

        // Start rover navigation/search logic here.

        send_ack();
    }

    void send_ack()
    {
        std_msgs::msg::String msg;
        msg.data = "ACK";

        ack_pub_->publish(msg);

        RCLCPP_INFO(get_logger(), "SearchArea ACK sent");
    }

    double latitude_ = 0.0;
    double longitude_ = 0.0;
    double radius_ = 0.0;

    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr
        command_sub_;

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr
        ack_pub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SearchAreaNode>());
    rclcpp::shutdown();
}
