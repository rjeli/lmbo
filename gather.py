import socket
import struct

UDP_IP = "0.0.0.0"
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

sensor_data = struct.Struct('= 4x 3f 36x d 51x')

while True:
    data, addr = sock.recvfrom(1024)
    x, y, z, c = sensor_data.unpack_from(data)
    print x, y, z, c
