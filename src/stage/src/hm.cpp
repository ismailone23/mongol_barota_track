#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/byte_multi_array.hpp"
#include "rscp.pb.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <sstream>

class HM : public rclcpp::Node
{
public:
    HM() : Node("hm_test")
    {
        request_publisher_ =
            create_publisher<std_msgs::msg::ByteMultiArray>(
                "/hm/request",
                10);

        input_thread_ =
            std::thread(&HM::keyboard_loop, this);
    }

    ~HM()
    {
        if (input_thread_.joinable())
        {
            input_thread_.join();
        }
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
                try
                {
                    const auto start = command.find('(');
                    const auto end = command.find(')', start + 1);

                    if (end == std::string::npos)
                    {
                        std::cout << "Malformed command\n";
                        continue;
                    }

                    const uint32_t stage =
                        std::stoul(
                            command.substr(
                                start + 1,
                                end - start - 1));

                    send_set_stage(stage);
                }
                catch (const std::exception &e)
                {
                    std::cout
                        << "Invalid SetStage command: "
                        << e.what()
                        << '\n';
                }
            }
            else if (command.rfind("SearchArea(", 0) == 0)
            {
                const auto start = command.find('(');
                const auto end = command.find(')', start + 1);

                if (start == std::string::npos ||
                    end == std::string::npos)
                {
                    std::cout << "Malformed SearchArea command\n";
                    continue;
                }

                std::string arguments =
                    command.substr(start + 1, end - start - 1);

                std::stringstream ss(arguments);

                std::string lat_str;
                std::string lon_str;
                std::string alt_str;
                std::string radius_str;

                if (!std::getline(ss, lat_str, ',') ||
                    !std::getline(ss, lon_str, ',') ||
                    !std::getline(ss, alt_str, ',') ||
                    !std::getline(ss, radius_str, ','))
                {
                    std::cout << "Invalid SearchArea arguments\n";
                    continue;
                }

                try
                {
                    double latitude = std::stod(lat_str);
                    double longitude = std::stod(lon_str);
                    float altitude = std::stof(alt_str);
                    float radius = std::stof(radius_str);

                    send_search_area(
                        latitude,
                        longitude,
                        altitude,
                        radius);
                }
                catch (const std::exception &e)
                {
                    std::cout
                        << "Invalid SearchArea values: "
                        << e.what()
                        << '\n';
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
            else if (command == "q")
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

    void publish_request(
        const rscp::RequestEnvelope &request)
    {
        std::string buffer;

        if (!request.SerializeToString(&buffer))
        {
            RCLCPP_ERROR(
                get_logger(),
                "Serialization failed");

            return;
        }

        std_msgs::msg::ByteMultiArray msg;

        msg.data.assign(
            buffer.begin(),
            buffer.end());

        request_publisher_->publish(msg);
    }

    void send_set_stage(uint32_t stage)
    {
        rscp::RequestEnvelope request;

        request.mutable_set_stage()->set_value(stage);

        publish_request(request);

        RCLCPP_INFO(
            get_logger(),
            "HM -> CM: SetStage(%u)",
            stage);
    }

    void send_arm_disarm(bool arm)
    {
        rscp::RequestEnvelope request;

        request.mutable_arm_disarm()->set_value(arm);

        publish_request(request);

        RCLCPP_INFO(
            get_logger(),
            "HM -> CM: ArmDisarm(arm=%s)",
            arm ? "True" : "False");
    }

    void send_search_area(
        double latitude,
        double longitude,
        float altitude,
        float radius)
    {
        rscp::RequestEnvelope request;

        auto *search = request.mutable_search_area();

        auto *center = search->mutable_center_coordinate();

        center->set_latitude(latitude);
        center->set_longitude(longitude);
        center->set_altitude(altitude);

        search->set_radius(radius);

        publish_request(request);

        RCLCPP_INFO(
            get_logger(),
            "HM -> CM: SearchArea(lat=%.6f, lon=%.6f, alt=%.2f, radius=%.2f)",
            latitude,
            longitude,
            altitude,
            radius);
    }
    rclcpp::Publisher<
        std_msgs::msg::ByteMultiArray>::SharedPtr
        request_publisher_;

    std::thread input_thread_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<HM>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}