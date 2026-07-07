#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "std_msgs/msg/string.hpp"
#include "rscp.pb.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

class CM : public rclcpp::Node
{
public:
    CM() : Node("communication_manager")
    {
        hm_request_subscriber_ =
            create_subscription<std_msgs::msg::ByteMultiArray>(
                "/hm/request",
                10,
                std::bind(
                    &CM::hm_request_callback,
                    this,
                    std::placeholders::_1));
        stage_publisher_ =
            create_publisher<std_msgs::msg::ByteMultiArray>(
                "/Stage",
                10);

        arm_disarm_publisher_ =
            create_publisher<std_msgs::msg::ByteMultiArray>(
                "/ArmDisarm",
                10);

        search_area_publisher_ =
            create_publisher<std_msgs::msg::ByteMultiArray>(
                "/SearchArea",
                10);

        acknowledge_subscriber_ =
            create_subscription<std_msgs::msg::String>(
                "/Acknowledge",
                10,
                std::bind(
                    &CM::acknowledge_callback,
                    this,
                    std::placeholders::_1));

        gps_subscriber_ =
            create_subscription<std_msgs::msg::ByteMultiArray>(
                "/GPSCoordinate",
                10,
                std::bind(
                    &CM::gps_callback,
                    this,
                    std::placeholders::_1));

        task_completed_subscriber_ =
            create_subscription<std_msgs::msg::ByteMultiArray>(
                "/TaskCompleted",
                10,
                std::bind(
                    &CM::task_completed_callback,
                    this,
                    std::placeholders::_1));

        RCLCPP_INFO(
            get_logger(),
            "Communication Manager started");
    }
    void hm_request_callback(
        const std_msgs::msg::ByteMultiArray::SharedPtr msg)
    {
        if (msg->data.empty())
        {
            RCLCPP_WARN(
                get_logger(),
                "Received empty HM request");

            return;
        }

        receive_from_hm(
            msg->data.data(),
            msg->data.size());
    }
    void receive_from_hm(
        const uint8_t *data,
        std::size_t size)
    {
        if (data == nullptr || size == 0)
        {
            RCLCPP_WARN(
                get_logger(),
                "Received empty HM buffer");

            return;
        }

        rscp::RequestEnvelope request;

        if (!request.ParseFromArray(
                data,
                static_cast<int>(size)))
        {
            RCLCPP_ERROR(
                get_logger(),
                "Failed to parse RequestEnvelope");

            return;
        }

        RCLCPP_INFO(
            get_logger(),
            "Valid RequestEnvelope received from HM");

        process_request(request);
    }

private:
    void process_request(
        const rscp::RequestEnvelope &request)
    {
        if (request.has_set_stage())
        {
            process_set_stage(request);
            return;
        }

        if (request.has_arm_disarm())
        {
            process_arm_disarm(request);
            return;
        }

        if (request.has_search_area())
        {
            process_search_area(request);
            return;
        }

        if (request.has_navigate_to_gps())
        {
            RCLCPP_WARN(
                get_logger(),
                "NavigateToGPS received but not implemented yet");

            return;
        }

        if (request.has_start_exploration())
        {
            RCLCPP_WARN(
                get_logger(),
                "StartExploration received but not implemented yet");

            return;
        }

        RCLCPP_WARN(
            get_logger(),
            "Unknown or empty RequestEnvelope");
    }

    void process_set_stage(
        const rscp::RequestEnvelope &request)
    {
        const uint32_t stage =
            request.set_stage().value();

        const std::string command =
            "SetStage(" +
            std::to_string(stage) +
            ")";

        RCLCPP_INFO(
            get_logger(),
            "HM -> CM: %s",
            command.c_str());

        publish_raw_command(
            stage_publisher_,
            command);

        RCLCPP_INFO(
            get_logger(),
            "CM -> Rover: %s",
            command.c_str());
    }

    void process_arm_disarm(
        const rscp::RequestEnvelope &request)
    {
        const bool arm =
            request.arm_disarm().value();

        const std::string command =
            arm
                ? "ArmDisarm(arm=True)"
                : "ArmDisarm(arm=False)";

        RCLCPP_INFO(
            get_logger(),
            "HM -> CM: %s",
            command.c_str());

        publish_raw_command(
            arm_disarm_publisher_,
            command);

        RCLCPP_INFO(
            get_logger(),
            "CM -> Rover: %s",
            command.c_str());
    }

    void process_search_area(
        const rscp::RequestEnvelope &request)
    {
        const auto &search = request.search_area();

        if (!search.has_center_coordinate())
        {
            RCLCPP_WARN(
                get_logger(),
                "SearchArea has no center coordinate");
            return;
        }

        const auto &center = search.center_coordinate();

        const double latitude = center.latitude();
        const double longitude = center.longitude();
        const float altitude = center.altitude();
        const float radius = search.radius();

        const std::string command =
            "SearchArea(" +
            std::to_string(latitude) + "," +
            std::to_string(longitude) + "," +
            std::to_string(altitude) + "," +
            std::to_string(radius) + ")";

        RCLCPP_INFO(
            get_logger(),
            "HM -> CM: %s",
            command.c_str());

        publish_raw_command(
            search_area_publisher_,
            command);

        RCLCPP_INFO(
            get_logger(),
            "CM -> Rover: %s",
            command.c_str());
    }
    void publish_raw_command(
        const rclcpp::Publisher<
            std_msgs::msg::ByteMultiArray>::SharedPtr &publisher,
        const std::string &command)
    {
        std_msgs::msg::ByteMultiArray message;

        message.data.assign(
            command.begin(),
            command.end());

        publisher->publish(message);
    }

    void acknowledge_callback(
        const std_msgs::msg::String::SharedPtr message)
    {
        (void)message;

        rscp::ResponseEnvelope response;
        response.mutable_acknowledge();

        std::string buffer;

        if (!response.SerializeToString(&buffer))
        {
            RCLCPP_WARN(get_logger(), "Failed to build Acknowledge response");
            return;
        }

        send_to_hm(
            reinterpret_cast<const uint8_t *>(buffer.data()),
            buffer.size());

        RCLCPP_INFO(
            get_logger(),
            "Rover -> CM: Acknowledge");

        RCLCPP_INFO(
            get_logger(),
            "CM -> HM: Acknowledge");
    }

    void gps_callback(
        const std_msgs::msg::ByteMultiArray::SharedPtr message)
    {
        rscp::ResponseEnvelope response;

        if (!parse_response(message, response))
        {
            RCLCPP_WARN(
                get_logger(),
                "Invalid response received on /GPSCoordinate");

            return;
        }

        if (!response.has_gps_coordinate())
        {
            RCLCPP_WARN(
                get_logger(),
                "/GPSCoordinate does not contain GPSCoordinate");

            return;
        }

        const auto &gps =
            response.gps_coordinate();

        RCLCPP_INFO(
            get_logger(),
            "Rover -> CM: GPSCoordinate(%.6f, %.6f)",
            gps.latitude(),
            gps.longitude());

        forward_response_to_hm(*message);

        RCLCPP_INFO(
            get_logger(),
            "CM -> HM: GPSCoordinate");
    }

    void task_completed_callback(
        const std_msgs::msg::ByteMultiArray::SharedPtr message)
    {
        rscp::ResponseEnvelope response;

        if (!parse_response(message, response))
        {
            RCLCPP_WARN(
                get_logger(),
                "Invalid response received on /TaskCompleted");

            return;
        }

        if (!response.has_task_finished())
        {
            RCLCPP_WARN(
                get_logger(),
                "/TaskCompleted does not contain TaskFinished");

            return;
        }

        RCLCPP_INFO(
            get_logger(),
            "Rover -> CM: TaskCompleted");

        forward_response_to_hm(*message);

        RCLCPP_INFO(
            get_logger(),
            "CM -> HM: TaskCompleted");
    }

    bool parse_response(
        const std_msgs::msg::ByteMultiArray::SharedPtr message,
        rscp::ResponseEnvelope &response)
    {
        if (message->data.empty())
        {
            return false;
        }

        return response.ParseFromArray(
            message->data.data(),
            static_cast<int>(message->data.size()));
    }

    void forward_response_to_hm(
        const std_msgs::msg::ByteMultiArray &message)
    {
        send_to_hm(
            message.data.data(),
            message.data.size());
    }

    void send_to_hm(
        const uint8_t *data,
        std::size_t size)
    {
        (void)data;

        RCLCPP_INFO(
            get_logger(),
            "Response buffer ready for HM: %zu bytes",
            size);
    }

    rclcpp::Publisher<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        stage_publisher_;

    rclcpp::Publisher<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        arm_disarm_publisher_;

    rclcpp::Publisher<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        search_area_publisher_;

    rclcpp::Subscription<
        std_msgs::msg::String>::SharedPtr
        acknowledge_subscriber_;

    rclcpp::Subscription<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        gps_subscriber_;

    rclcpp::Subscription<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        task_completed_subscriber_;

    rclcpp::Subscription<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        hm_request_subscriber_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto cm = std::make_shared<CM>();
    rclcpp::spin(cm);

    rclcpp::shutdown();

    return 0;
}