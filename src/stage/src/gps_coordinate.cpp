#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "rscp.pb.h"

#include <iostream>
#include <thread>

class GPSNode : public rclcpp::Node
{
public:
    GPSNode() : Node("gps_coordinate_node")
    {
        gps_pub_ =
            create_publisher<std_msgs::msg::ByteMultiArray>(
                "/GPSCoordinate",
                10);

        input_thread_ =
            std::thread(&GPSNode::keyboard_loop, this);
    }

    ~GPSNode()
    {
        if (input_thread_.joinable())
            input_thread_.join();
    }

private:
    void keyboard_loop()
    {
        while (rclcpp::ok())
        {
            std::cout << "Type gps to send GPSCoordinate: ";

            std::string command;
            std::getline(std::cin, command);

            if (command == "gps")
            {
                send_gps(
                    23.810331,
                    90.412521);
            }
        }
    }

    void send_gps(double latitude, double longitude)
    {
        rscp::ResponseEnvelope response;

        auto *gps = response.mutable_gps_coordinate();

        gps->set_latitude(latitude);
        gps->set_longitude(longitude);

        std::string buffer;

        if (!response.SerializeToString(&buffer))
            return;

        std_msgs::msg::ByteMultiArray msg;
        msg.data.assign(buffer.begin(), buffer.end());

        gps_pub_->publish(msg);

        RCLCPP_INFO(
            get_logger(),
            "GPSCoordinate sent: %.6f %.6f",
            latitude,
            longitude);
    }

    rclcpp::Publisher<std_msgs::msg::ByteMultiArray>::SharedPtr
        gps_pub_;

    std::thread input_thread_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GPSNode>());
    rclcpp::shutdown();
}