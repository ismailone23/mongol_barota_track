#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include <cmath>
#include "rscp.pb.h"

constexpr double REF_LAT = 23.839893;
constexpr double REF_LON = 90.358991;

struct XY
{
    double x, y;
}; // x = east (m), y = north (m)

XY latlon_to_local_xy(double lat_deg, double lon_deg,
                      double ref_lat_deg, double ref_lon_deg)
{
    constexpr double R = 6378137.0; // Earth radius (m), WGS84 approx
    double lat = lat_deg * M_PI / 180.0;
    double lon = lon_deg * M_PI / 180.0;
    double ref_lat = ref_lat_deg * M_PI / 180.0;
    double ref_lon = ref_lon_deg * M_PI / 180.0;

    XY p;
    p.x = R * (lon - ref_lon) * std::cos(ref_lat); // east
    p.y = R * (lat - ref_lat);                     // north
    return p;
}

class MinSubscriber : public rclcpp::Node
{
public:
    MinSubscriber() : Node("rscp_receiver")
    {
        subscriber_ = this->create_subscription<std_msgs::msg::ByteMultiArray>("/gps",
                                                                               10,
                                                                               std::bind(&MinSubscriber::listener_callback,
                                                                                         this, std::placeholders::_1));
    }

private:
    void listener_callback(const std_msgs::msg::ByteMultiArray::SharedPtr msg)
    {
        std::string serialized(
            msg->data.begin(),
            msg->data.end());

        rscp::GPSCoordinate gps;

        if (!gps.ParseFromString(serialized))
        {
            RCLCPP_ERROR(this->get_logger(),
                         "Failed to parse packet");
            return;
        }

        double lat, lon;

        lat = gps.latitude();
        lon = gps.longitude();

        XY cordinate = latlon_to_local_xy(lat, lon, REF_LAT, REF_LON);

        RCLCPP_INFO(this->get_logger(), "x: %lf, y: %lf", cordinate.x, cordinate.y);
    }
    rclcpp::Subscription<std_msgs::msg::ByteMultiArray>::SharedPtr subscriber_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MinSubscriber>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}