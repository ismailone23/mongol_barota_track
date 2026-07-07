#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "std_msgs/msg/string.hpp"
#include "rscp.pb.h"

#include <sstream>
#include <string>

class SearchAreaNode : public rclcpp::Node
{
public:
    SearchAreaNode() : Node("search_area_node")
    {
        command_sub_ =
            create_subscription<std_msgs::msg::ByteMultiArray>(
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
        const std_msgs::msg::ByteMultiArray::SharedPtr msg)
    {
        std::string command(msg->data.begin(), msg->data.end());

        if (command.rfind("SearchArea(", 0) != 0)
        {
            RCLCPP_WARN(get_logger(), "Invalid SearchArea command");
            return;
        }

        const auto start = command.find('(');
        const auto end = command.find(')');

        if (start == std::string::npos ||
            end == std::string::npos)
        {
            RCLCPP_WARN(get_logger(), "Malformed SearchArea");
            return;
        }

        std::string arguments =
            command.substr(start + 1, end - start - 1);

        std::stringstream ss(arguments);

        std::string lat_string;
        std::string lon_string;
        std::string radius_string;

        if (!std::getline(ss, lat_string, ',') ||
            !std::getline(ss, lon_string, ',') ||
            !std::getline(ss, radius_string, ','))
        {
            RCLCPP_WARN(get_logger(), "Invalid SearchArea arguments");
            return;
        }

        try
        {
            latitude_ = std::stod(lat_string);
            longitude_ = std::stod(lon_string);
            radius_ = std::stof(radius_string);
        }
        catch (...)
        {
            RCLCPP_ERROR(get_logger(), "Invalid SearchArea values");
            return;
        }

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
    float radius_ = 0.0F;

    rclcpp::Subscription<std_msgs::msg::ByteMultiArray>::SharedPtr
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