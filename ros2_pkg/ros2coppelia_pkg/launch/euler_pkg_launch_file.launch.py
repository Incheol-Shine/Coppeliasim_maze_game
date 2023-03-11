from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='ros2coppelia_pkg',
            executable='euler_angle_pub',
            output='screen'),
    ])