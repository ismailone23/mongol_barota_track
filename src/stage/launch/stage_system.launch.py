from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package="stage",
            executable="cm",
            name="communication_manager",
            output="screen",
        ),

        Node(
            package="stage",
            executable="stage_node",
            name="stage_node",
            output="screen",
        ),

        Node(
            package="stage",
            executable="arm_disarm",
            name="arm_disarm_node",
            output="screen",
        ),

        Node(
            package="stage",
            executable="search_area",
            name="search_area_node",
            output="screen",
        ),

        Node(
            package="stage",
            executable="gps_coordinate",
            name="gps_coordinate_node",
            output="screen",
        ),

        Node(
            package="stage",
            executable="task_completed",
            name="task_completed_node",
            output="screen",
        ),
    ])