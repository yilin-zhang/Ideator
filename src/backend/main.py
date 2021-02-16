from typing import List, Any, Tuple
from pathlib import Path
import pickle
import re
import os

import torch
import numpy as np

from pythonosc import udp_client
from pythonosc import osc_server
from pythonosc import dispatcher

from audio_feature_extractor import AudioFeatureExtractor
from receive_audio_buffer import UdpBufferReceiver


class LibraryReceiver:
    def __init__(self, feature_extractor: AudioFeatureExtractor):
        self._num_parameters = 0
        self._library_info = {}
        self._feature_extractor = feature_extractor

    # manage the library data construction
    def add_library_info(self, preset_path: str, descriptors: Tuple[str], buffer: np.array) -> None:
        print(f'preset_path: {preset_path}')
        print(f'descriptor_list: {descriptors}')
        buffer = torch.from_numpy(buffer)
        buffer = buffer.reshape((1, -1))
        preset_feature = self._feature_extractor.encode(buffer)
        self._library_info[preset_path] = {'feature': preset_feature, 'descriptors': descriptors}

    def save_library_info(self) -> None:
        # TODO: the cache directory is in backend/ folder, consider move it to user directory in the future
        # this is a relative path, do NOT run this script when you are not in backend/ folder
        Path('./cache').mkdir(parents=True, exist_ok=True)
        # dump the data
        with open('./cache/preset_lib.pkl', 'wb') as f:
            pickle.dump(self._library_info, f)


class PresetRetriever:
    def __init__(self, cache_path: str):
        # Load the cache. If there is no cache file, the member variables are empty
        self._preset_paths = []
        self._preset_descriptors = []
        self._feature_matrix = []
        self._feature_matrix = np.array([])
        self.load_cache(cache_path)

    def retrieve_presets(self, keywords: str) -> List[str]:
        # split the keyword string
        keyword_list = re.split('[^a-zA-Z]+', keywords)
        # simply find the presets that contains the keywords
        selected_paths = []
        for idx, descriptor_list in enumerate(self._preset_descriptors):
            if all(word in descriptor_list for word in keyword_list):
                selected_paths.append(self._preset_paths[idx])
        return selected_paths

    def load_cache(self, cache_path: str) -> None:
        if os.path.isfile(cache_path):
            with open(cache_path, 'rb') as f:
                library_info = pickle.load(f)
            feature_list = []
            for preset_path in library_info:
                self._preset_paths.append(preset_path)
                self._preset_descriptors.append(library_info[preset_path]['descriptors'])
                feature = library_info[preset_path]['feature']
                feature_list.append(feature.reshape(feature.shape[1]))
            self._feature_matrix = torch.tensor(feature_list).numpy()


def analyze_library_callback(address: str,
                             args: List[Any],
                             *osc_args: List[Any]) -> None:
    client, library_receiver, udp_buffer_receiver = args
    value = osc_args[0]
    preset_path = osc_args[1]
    descriptors = osc_args[2]

    # receive an audio buffer
    if value == 1:
        buffer = udp_buffer_receiver.receive()

        descriptor_list = tuple(descriptors.split(','))
        library_receiver.add_library_info(preset_path, descriptor_list, buffer)
        client.send_message("/Ideator/cpp/analyze_library", 1)

    # data receiving is over, save the library data
    elif value == 2:
        print('All presets received, dumping...')
        library_receiver.save_library_info()
        print('Finished')


def retrieve_presets_callback(address: str,
                              args: List[Any],
                              *osc_args: List[Any]) -> None:
    client, preset_retriever = args
    tag_string = str(osc_args[0])
    selected_paths = preset_retriever.retrieve_presets(tag_string)

    client.send_message('/Ideator/cpp/retrieve_presets/start', 1)
    for path in selected_paths:
        client.send_message('/Ideator/cpp/retrieve_presets/send', path)
    client.send_message('/Ideator/cpp/retrieve_presets/end', 1)


if __name__ == "__main__":
    dispatcher = dispatcher.Dispatcher()
    client = udp_client.SimpleUDPClient(address="127.0.0.1", port=9001)
    feature_extractor = AudioFeatureExtractor('models/auto-encoder-20201020050003.pt') # NOTE: hard coded here
    udp_buffer_receiver = UdpBufferReceiver(address="127.0.0.1", port=8888)
    library_receiver = LibraryReceiver(feature_extractor)
    preset_retriever = PresetRetriever('./cache/preset_lib.pkl')

    dispatcher.map("/Ideator/python/analyze_library", analyze_library_callback, client, library_receiver, udp_buffer_receiver)
    dispatcher.map("/Ideator/python/retrieve_presets", retrieve_presets_callback, client, preset_retriever)

    server = osc_server.ThreadingOSCUDPServer(("127.0.0.1", 7777), dispatcher)
    print("Serving on {}".format(server.server_address))
    server.serve_forever()
