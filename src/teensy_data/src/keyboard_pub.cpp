#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <termios.h>
#include <unistd.h>

class KeyboardPub : public rclcpp::Node
{
public:
    KeyboardPub() : Node("keyboard_pub")
    {
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/teensy_data", 10);
        enable_row_mode();
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&KeyboardPub::read_key, this));

        RCLCPP_INFO(this->get_logger(), "Keyboard ready. Use w/a/s/d to move, x to stop");
    }

    ~KeyboardPub()
    {
        disable_row_mode();
    }

private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    struct termios original_tty_;
    rclcpp::TimerBase::SharedPtr timer_;

    void read_key()
    {
        char key;
        if (read(STDIN_FILENO, &key, 1) < 1)
            return;
        auto msg = geometry_msgs::msg::Twist();

        switch (key)
        {
        case 'w':
            msg.linear.x = 1.0;
            msg.angular.z = 0.0;
            break;
        case 's':
            msg.linear.x = -1.0;
            msg.angular.z = 0.0;
            break;
        case 'a':
            msg.linear.x = 0.0;
            msg.angular.z = 1.0;
            break;
        case 'd':
            msg.linear.x = 0.0;
            msg.angular.z = -1.0;
            break;
        case 'x':
            msg.linear.x = 0.0;
            msg.angular.z = 0.0;
            break;
        default:
            return;
        }
        RCLCPP_INFO(this->get_logger(), "Key: %c | linear=%.1f angular=%.1f",
                    key, msg.linear.x, msg.angular.z);
        publisher_->publish(msg);
    }
    void enable_row_mode()
    {
        struct termios raw;
        tcgetattr(STDIN_FILENO, &original_tty_);
        raw = original_tty_;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    void disable_row_mode()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_tty_);
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyboardPub>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
