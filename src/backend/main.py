from typing import List, Any
import torch
import numpy as np

from pythonosc import udp_client
from pythonosc import osc_server
from pythonosc import dispatcher

from pathlib import Path
import pickle

from audio_feature_extractor import AudioFeatureExtractor
from receive_audio_buffer import UdpBufferReceiver


class LibraryReceiver:
    def __init__(self, feature_extractor: AudioFeatureExtractor):
        self._num_parameters = 0
        self._library_info = {}
        self._feature_extractor = feature_extractor

    # manage the library data construction
    def add_library_info(self, preset_path: str, buffer: np.array) -> None:
        buffer = torch.from_numpy(buffer)
        buffer = buffer.reshape((1, -1))
        preset_feature = self._feature_extractor.encode(buffer)
        print(f'{preset_feature.size}')
        self._library_info[preset_path] = preset_feature

    def save_library_info(self) -> None:
        # TODO: the cache directory is in backend/ folder, consider move it to user directory in the future
        # this is a relative path, do NOT run this script when you are not in backend/ folder
        Path('./cache').mkdir(parents=True, exist_ok=True)
        # dump the data
        with open('./cache/preset_lib.pkl', 'wb') as f:
            pickle.dump(self._library_info, f)


def analyze_library_callback(address: str, args: List[Any], *osc_args: List[Any]) -> None:
    client, library_receiver, udp_buffer_receiver = args
    value = osc_args[0]
    preset_path = osc_args[1]

    # receive an audio buffer
    if value == 1:
        print(f'preset_path: {preset_path}')
        buffer = udp_buffer_receiver.receive()
        library_receiver.add_library_info(preset_path, buffer)
        client.send_message("/Ideator/cpp/analyze_library", 1)

    # data receiving is over, save the library data
    elif value == 2:
        print('All presets received, dumping...')
        library_receiver.save_library_info()
        print('Finished')


if __name__ == "__main__":
    dispatcher = dispatcher.Dispatcher()
    client = udp_client.SimpleUDPClient(address="127.0.0.1", port=9001)
    feature_extractor = AudioFeatureExtractor('models/auto-encoder-20201020050003.pt') # NOTE: hard coded here
    udp_buffer_receiver = UdpBufferReceiver(address="127.0.0.1", port=8888)
    library_receiver = LibraryReceiver(feature_extractor)

    dispatcher.map("/Ideator/python/analyze_library", analyze_library_callback, client, library_receiver, udp_buffer_receiver)

    server = osc_server.ThreadingOSCUDPServer(("127.0.0.1", 7777), dispatcher)
    print("Serving on {}".format(server.server_address))
    server.serve_forever()
