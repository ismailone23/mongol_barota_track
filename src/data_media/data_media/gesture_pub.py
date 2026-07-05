import rclpy
from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from geometry_msgs.msg import Twist

from data_media.camera import Camera
from data_media.detector import HandDetector

class GesturePublisher(Node):
    def __init__(self):
        super().__init__('gesture_publisher')

        self.publisher_ = self.create_publisher(Twist, '/teensy_data', 10)
        
        self.camera = Camera(0)
        self.detector = HandDetector(
            "/home/ismail/Documents/ros2_base/src/data_media/data_media/hand_landmarker.task"
        )

        self.timer = self.create_timer(
            0.05,
            self.process_frame
        )
    def process_frame(self):

        frame = self.camera.read()

        if frame is None:
            return

        hands = self.detector.process(frame)

        if not hands:
            return

        gesture = hands[0]["gesture"]

        msg = Twist()

        if gesture == "Point":
            msg.linear.x = 1.0

        elif gesture == "Fist" or gesture == "Open Hand":
            msg.linear.x = 0.0
            msg.angular.z = 0.0

        elif gesture == "Peace":
            msg.angular.z = 1.0

        elif gesture == "Three":
            msg.angular.z = -1.0
        elif gesture == "Thumbs Up":
            msg.linear.x = 0.0
            msg.angular.z = 0.0

        self.publisher_.publish(msg)
    

def main(args=None):
    try:
        rclpy.init()
        node = GesturePublisher()
        rclpy.spin(node)

    except (KeyboardInterrupt,ExternalShutdownException):
        pass

    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()