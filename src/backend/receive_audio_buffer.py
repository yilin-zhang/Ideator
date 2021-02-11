import socket
import numpy as np
import struct
from scipy.io.wavfile import write as wavwrite


class UdpBufferReceiver:
    def __init__(self, address: str, port: int):
        self._address = address
        self._port = port
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._socket.bind((self._address, self._port))

    def receive(self) -> np.array:
        meta_fmt = 'ii?i'
        meta_size = struct.calcsize(meta_fmt)

        msg_buf = {}
        num_msgs = 0
        last_received = False

        while True:
            data, addr = self._socket.recvfrom(512)
            msg_id, idx, is_last, num_samples = struct.unpack(meta_fmt, data[:meta_size])
            msg_buf[idx] = (data[meta_size:], num_samples)

            # check if the all the messages have been received once
            # the last one has been received
            if is_last or last_received:
                if is_last:
                    last_received = True
                    num_msgs = idx + 1
                if len(msg_buf) == num_msgs:
                    break

        msg_list = [msg_buf[i] for i in range(num_msgs)]

        data_list = []
        for msg in msg_list:
            data, num_samples = msg
            data_list.append(np.frombuffer(data, dtype=np.float32)[:num_samples])

        return np.concatenate(data_list)

    @property
    def address(self) -> str:
        return self._address

    @property
    def port(self) -> int:
        return self._port


if __name__ == '__main__':
    UDP_IP = '127.0.0.1'
    UDP_PORT = 8888
    receiver = UdpBufferReceiver(UDP_IP, UDP_PORT)
    audio = receiver.receive()
    wavwrite('test_audio/test.wav', 44100, audio)
