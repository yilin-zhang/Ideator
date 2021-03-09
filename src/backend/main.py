from typing import List, Any
import re

import torch

from pythonosc import udp_client
from pythonosc import osc_server
from pythonosc import dispatcher

from audio_feature_extractor import AudioFeatureExtractor
from receive_audio_buffer import UdpBufferReceiver
from preset_retriever import LibraryReceiver, PresetRetriever


def analyze_library_callback(address: str,
                             args: List[Any],
                             *osc_args: List[Any]) -> None:
    client, library_receiver, udp_buffer_receiver, preset_retriever = args
    value = osc_args[0]
    preset_path = osc_args[1]
    descriptors = osc_args[2]

    # receive an audio buffer
    if value == 1:
        buffer = udp_buffer_receiver.receive()

        descriptor_list = re.split('[^a-zA-Z]+', descriptors)
        library_receiver.add_library_info(preset_path, descriptor_list, buffer)
        client.send_message("/Ideator/cpp/analyze_library", 1)

    # data receiving is over, save the library data
    elif value == 2:
        print('All presets received, dumping...')
        library_receiver.save_library_info()
        preset_retriever.load_cache('./cache/preset_lib.pkl')
        print('Finished')


def find_similar_callback(address: str,
                          args: List[Any],
                          *osc_args: List[Any]) -> None:
    client, udp_buffer_receiver, feature_extractor, preset_retriever = args
    value = osc_args[0]
    # receive a buffer
    buffer = udp_buffer_receiver.receive()
    buffer = torch.from_numpy(buffer)
    buffer = buffer.reshape((1, -1))
    # extract latent features
    preset_feature = feature_extractor.encode(buffer)
    preset_feature = preset_feature.reshape(preset_feature.shape[1])
    # retrieve presets
    selected_paths = preset_retriever.retrieve_presets_by_features(preset_feature)

    client.send_message('/Ideator/cpp/retrieve_presets/start', 1)
    for path in selected_paths:
        client.send_message('/Ideator/cpp/retrieve_presets/send', path)
    client.send_message('/Ideator/cpp/retrieve_presets/end', 1)


def retrieve_presets_callback(address: str,
                              args: List[Any],
                              *osc_args: List[Any]) -> None:
    client, preset_retriever = args
    tag_string = str(osc_args[0])
    # split the keyword string
    keyword_list = re.split('[^a-zA-Z]+', tag_string)
    selected_paths = preset_retriever.retrieve_presets_by_keywords(keyword_list)

    client.send_message('/Ideator/cpp/retrieve_presets/start', 1)
    for path in selected_paths:
        client.send_message('/Ideator/cpp/retrieve_presets/send', path)
    client.send_message('/Ideator/cpp/retrieve_presets/end', 1)


def auto_tag_callback(address: str,
                      args: List[Any],
                      *osc_args: List[Any]) -> None:
    client, udp_buffer_receiver, preset_retriever = args
    value = osc_args[0]
    buffer = udp_buffer_receiver.receive()
    buffer = torch.from_numpy(buffer)
    buffer = buffer.reshape((1, -1))
    # extract latent features
    preset_feature = feature_extractor.encode(buffer)
    preset_feature = preset_feature.reshape(preset_feature.shape[1])
    # auto_tag
    tags = preset_retriever.auto_tag(preset_feature)
    # send message
    client.send_message('/Ideator/cpp/auto_tag', tags)


def change_descriptors_callback(address: str,
                                args: List[Any],
                                *osc_args: List[Any]) -> None:
    client, preset_retriever = args
    preset_path = str(osc_args[0])
    tag_string = str(osc_args[1])
    print(f'preset_path: {preset_path}')
    # split the keyword string
    keyword_list = re.split('[^a-zA-Z]+', tag_string)
    preset_retriever.change_descriptors(preset_path, keyword_list)


if __name__ == "__main__":
    dispatcher = dispatcher.Dispatcher()
    client = udp_client.SimpleUDPClient(address="127.0.0.1", port=9001)
    feature_extractor = AudioFeatureExtractor('models/auto-encoder-20201020050003.pt') # NOTE: hard coded here
    udp_buffer_receiver = UdpBufferReceiver(address="127.0.0.1", port=8888)
    library_receiver = LibraryReceiver(feature_extractor)
    preset_retriever = PresetRetriever('./cache/preset_lib.pkl')

    dispatcher.map("/Ideator/python/analyze_library", analyze_library_callback,
                   client, library_receiver, udp_buffer_receiver, preset_retriever)
    dispatcher.map("/Ideator/python/retrieve_presets", retrieve_presets_callback, client, preset_retriever)
    dispatcher.map("/Ideator/python/find_similar", find_similar_callback, client,
                   udp_buffer_receiver, feature_extractor, preset_retriever)
    dispatcher.map("/Ideator/python/auto_tag", auto_tag_callback, client, udp_buffer_receiver, preset_retriever)
    dispatcher.map("/Ideator/python/change_descriptors", change_descriptors_callback, client, preset_retriever)

    server = osc_server.ThreadingOSCUDPServer(("127.0.0.1", 7777), dispatcher)
    print("Serving on {}".format(server.server_address))
    server.serve_forever()
