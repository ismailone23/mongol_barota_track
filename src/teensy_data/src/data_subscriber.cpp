#include "rclcpp/rclcpp.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include "geometry_msgs/msg/twist.hpp"

class DataSubs : public rclcpp::Node
{
public:
    DataSubs() : Node("data_subs")
    {
        subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>("/teensy_data",
                                                                           10,
                                                                           std::bind(&DataSubs::timer_callback,
                                                                                     this,
                                                                                     std::placeholders::_1));
        serialpd_ = open_serial("/dev/ttyACM0", B115200);
    }

    ~DataSubs()
    {
        if (serialpd_ > 0)
            close(serialpd_);
    }

private:
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscriber_;
    int serialpd_;
    void timer_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Got linear: %.2f, angular: %.2f", msg->linear.x, msg->angular.z);
        if (serialpd_ < 0)
        {
            RCLCPP_WARN(this->get_logger(), "Serial port not open, skipping write");
            return;
        }
        char buf[64];
        int len = snprintf(buf, sizeof(buf), "L:%.2f,A:%.2f\n", msg->linear.x, msg->angular.z);
        int bytes = write(serialpd_, buf, len);
        if (bytes < 0)
        {
            RCLCPP_ERROR(this->get_logger(), "Serial write failed");
        }
    }

    int open_serial(const char *port, speed_t baud)
    {
        int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0)
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to open port: %s", port);
            return -1;
        }
        struct termios tty;
        tcgetattr(fd, &tty);
        cfsetspeed(&tty, baud);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_cflag |= (CLOCAL | CREAD);   // enable receiver
        tty.c_cflag &= ~(PARENB | PARODD); // no parity
        tty.c_cflag &= ~CSTOPB;            // 1 stop bit
        tty.c_iflag = IGNPAR;              // ignore parity errors
        tty.c_oflag = 0;                   // raw output
        tty.c_lflag = 0;                   // raw input
        tcsetattr(fd, TCSANOW, &tty);
        return fd;
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<DataSubs>();
    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}