import zmq
import sys


context = zmq.Context()
socket = context.socket(zmq.SUB)

print "hello"
socket.connect("tcp://localhost:5565")

