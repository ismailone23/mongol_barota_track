#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "rscp.pb.h"
#include <iomanip>
#include <sstream>

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher() : Node("rscp_sender")
    {
        publisher_ = this->create_publisher<std_msgs::msg::ByteMultiArray>("/gps", 10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&MinimalPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
        rscp::GPSCoordinate gps;

        gps.set_latitude(23.810331);
        gps.set_longitude(90.412521);
        gps.set_altitude(15.5f);

        std::string serialized;

        if (gps.SerializeToString(&serialized))
        {
            std_msgs::msg::ByteMultiArray msg;

            msg.data.assign(serialized.begin(), serialized.end());

            publisher_->publish(msg);

            RCLCPP_INFO(
                this->get_logger(),
                "Published RSCP packet (%zu bytes)",
                serialized.size());
        }
    }
    rclcpp::Publisher<std_msgs::msg::ByteMultiArray>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<MinimalPublisher>();
    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}