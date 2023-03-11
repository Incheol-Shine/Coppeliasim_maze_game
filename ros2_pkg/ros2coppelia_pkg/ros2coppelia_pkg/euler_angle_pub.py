import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Vector3
import	serial
import	struct
from	typing	import Final

""" constant """
DATA_START:	Final = 0xAA
DATA_END:	Final = 0xDD
DATA_SIZE:	Final = 16
DEG_2_RAD:	Final =  0.01745329251994329576923690768489
ALPHA:		Final = 0.5

class SerialReceiver:
	def __init__(self):
		self.euler_x = 0
		self.euler_y = 0
		self.mean_x = 0
		self.mean_y = 0
		self.button = 0

class SimplePublisher(Node):
	def __init__(self):
		super().__init__('euler_angle_pub')
		self.publisher_ = self.create_publisher(Vector3, 'euler_angle', 10)

def read_data(ser, receiver):
	if ser.readable():
		if (ser.in_waiting >= DATA_SIZE):
			data = ser.read()
			if data[0] != DATA_START:
				return
			data = ser.read(DATA_SIZE - 1)
			if data[-1] != DATA_END:
				return
			else:
				receiver.euler_x = struct.unpack('f', data[0:4])[0]
				receiver.euler_y = struct.unpack('f', data[4:8])[0]
				receiver.mean_x = receiver.mean_x * ALPHA + receiver.euler_x * (1 - ALPHA)
				receiver.mean_y = receiver.mean_y * ALPHA + receiver.euler_y * (1 - ALPHA)
				receiver.is_move = data[12]
				receiver.button = data[13]

def main(args=None):
	ser = serial.Serial("/dev/ttyUSB0", 115200)
	receiver = SerialReceiver()
	# initialize the ROS communication
	rclpy.init(args=args)
	# declare the node constructor
	simple_publisher = SimplePublisher()
	msg = Vector3()
	while rclpy.ok():
		read_data(ser, receiver)
		msg.x = float(receiver.mean_x)
		msg.y = float(receiver.mean_y)
		print(f"x:{msg.x}, y:{msg.y}")
		simple_publisher.publisher_.publish(msg)
	# Explicity destroys the node
	simple_publisher.destroy_node()
	# shutdown the ROS communication
	rclpy.shutdown()

if __name__ == '__main__':
	main()