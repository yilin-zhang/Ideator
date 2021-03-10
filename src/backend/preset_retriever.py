from typing import Tuple, List
from pathlib import Path
from collections import defaultdict, Counter
import pickle
import os
import bcolz
import numpy as np
import torch

from audio_feature_extractor import AudioFeatureExtractor


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
        self._glove = {}
        self.load_cache(cache_path)
        self.load_glove()

    def retrieve_presets_by_keywords(self, keyword_list: List[str]) -> List[str]:
        input_vector_list = [self._glove[keyword.lower()] for keyword in keyword_list]

        # Match each input keyword to every descriptor in a preset and find the max similarity
        # value, and take the sum of the max similarities.
        weights = []
        for idx, descriptor_list in enumerate(self._preset_descriptors):
            weight = 0
            for input_vector in input_vector_list:
                max_sim = max([self._dist_similarity(input_vector, self._glove[descriptor.lower()])
                               for descriptor in descriptor_list])
                weight += max_sim
            weights.append(weight)
        weights = np.array(weights)

        # If the word is not presented is the database, usually the difference between the max value and the min value
        # is small (less than 1), which is not good for selecting the ones that have higher weights.
        # So the values are non-linearly scaled in order to have a bigger difference after the normalization
        if max(weights) - min(weights) < 1:
            weights = pow(weights, 5)

        # randomly pick 5 presets according to the weights
        weights = (weights - weights.min()) / (weights - weights.min()).sum()  # normalize it to make it sum to 1
        randomly_picked_ind = np.random.choice(np.arange(len(weights)), size=5, p=weights)
        selected_paths = [self._preset_paths[i] for i in randomly_picked_ind]

        return selected_paths

    def retrieve_presets_by_features(self, latent: np.array) -> List[str]:
        dist = np.linalg.norm(latent - self._feature_matrix, axis=1)
        min_dists_ind = list(np.argsort(dist)[:5])
        selected_paths = [self._preset_paths[i] for i in min_dists_ind]
        return selected_paths

    def auto_tag(self, latent: np.array) -> List[str]:
        # KNN algorithm, returns the tags
        k = 10
        dist = np.linalg.norm(latent - self._feature_matrix, axis=1)
        min_dists_ind = list(np.argsort(dist)[:k])
        selected_descriptors = [self._preset_descriptors[i] for i in min_dists_ind]
        selected_descriptors = [descriptor for subl in selected_descriptors for descriptor in subl] # flatten
        # find the most frequent descriptors
        freq_dict = dict(Counter(selected_descriptors))
        sorted_descriptors = sorted(freq_dict, key=freq_dict.get, reverse=True)
        # take 3 descriptors, otherwise take all
        if len(sorted_descriptors) >= 3:
            return sorted_descriptors[:3]
        else:
            return sorted_descriptors

    def change_descriptors(self, path: str, descriptors: List[str]) -> None:
        descriptors = list(map(lambda x: x.capitalize(), descriptors))  # make sure the words are capitalized
        idx = self._preset_paths.index(path)
        self._preset_descriptors[idx] = descriptors
        print(f'new descriptors: {descriptors}')
        self.save_cache('./cache/preset_lib.pkl')

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

    def save_cache(self, cache_path: str) -> None:
        library_info = {}
        # split the feature matrix into a feature list
        feature_list = []
        for i in range(self._feature_matrix.shape[0]):
            feature_list.append(self._feature_matrix[i, :].reshape(1, -1))

        for path, descriptors, feature in zip(self._preset_paths, self._preset_descriptors, feature_list):
            library_info[path] = {'descriptors': descriptors, 'feature': feature}

        Path(os.path.dirname(cache_path)).mkdir(parents=True, exist_ok=True)
        # dump the data
        with open(cache_path, 'wb') as f:
            pickle.dump(library_info, f)

    def load_glove(self) -> None:
        glove_path = './glove'
        word_vectors = bcolz.open(f'{glove_path}/6B.50.dat')[:]
        words = pickle.load(open(f'{glove_path}/6B.50_words.pkl', 'rb'))
        word2idx = pickle.load(open(f'{glove_path}/6B.50_idx.pkl', 'rb'))
        self._glove = defaultdict(lambda: np.ones(50) * np.sqrt(1/50), {w: word_vectors[word2idx[w]] for w in words})

    @staticmethod
    def _cosine_similarity(a: np.array, b: np.array) -> float:
        cos_sim = np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b))
        return cos_sim

    @staticmethod
    def _dist_similarity(a: np.array, b: np.array) -> float:
        dist = np.linalg.norm(a - b)
        if dist < pow(10, -5):  # make sure the value is above 0
            dist = pow(10, -5)
        return 1 / dist
