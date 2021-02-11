import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.tensorboard import SummaryWriter
from torch.utils.data import Dataset, DataLoader
import pickle
import numpy as np
from tqdm import trange
from pathlib import Path
from datetime import datetime
import sys


class MelDataset(Dataset):
    def __init__(self, flat=False, normalize=True):
        with open('dataset/mel.pkl', 'rb') as f:
            features = pickle.load(f)

        with open('dataset/labels.pkl', 'rb') as f:
            labels = pickle.load(f)

        self.labels = labels

        if normalize:
            features = torch.log(features+1)
            maxes, _ = torch.max(features.view(features.size(0), -1), dim=1)
            maxes = maxes.view(maxes.size(0), 1, 1)
            features /= maxes

        if flat:
            self.feature = torch.reshape(
                features,
                shape=(features.size(0),
                       features.size(1) * features.size(2)))
        else:
            self.features = torch.reshape(features, (features.size(0),
                                                     1,
                                                     features.size(1),
                                                     features.size(2)))

        with open('dataset/paths.pkl', 'rb') as f:
            self.paths = pickle.load(f)

    def __getitem__(self, index):
        return self.features[index], self.labels[index], self.paths[index]

    def __len__(self):
        return self.features.size(0)


# DNN (linear)
class DivaDNN(nn.Module):
    def __init__(self):
        super(DivaDNN, self).__init__()
        self.lin1 = nn.Linear(8320, 4096, bias=True)
        self.bn1 = nn.BatchNorm1d(4096)
        self.lin2 = nn.Linear(4096, 2048, bias=True)
        self.bn2 = nn.BatchNorm1d(2048)
        self.lin3 = nn.Linear(2048, 1024, bias=True)
        self.bn3 = nn.BatchNorm1d(1024)
        self.lin4 = nn.Linear(1024, 512, bias=True)
        self.bn4 = nn.BatchNorm1d(512)
        self.lin5_1 = nn.Linear(512, 3, bias=True)
        self.lin5_2 = nn.Linear(512, 3, bias=True)
        self.lin5_3 = nn.Linear(512, 3, bias=True)
        self.lin5_4 = nn.Linear(512, 3, bias=True)
        self.lin5_5 = nn.Linear(512, 3, bias=True)
        self.lin5_6 = nn.Linear(512, 3, bias=True)
        self.lin5_7 = nn.Linear(512, 3, bias=True)
        self.lin5_8 = nn.Linear(512, 3, bias=True)
        self.lin5_9 = nn.Linear(512, 3, bias=True)
        self.lin5_10 = nn.Linear(512, 3, bias=True)
        self.lin5_11 = nn.Linear(512, 3, bias=True)
        self.softmax = nn.Softmax(dim=1)

    def forward(self, x):
        x = F.relu(self.bn1(self.lin1(x)))
        x = F.relu(self.bn2(self.lin2(x)))
        x = F.relu(self.bn3(self.lin3(x)))
        x = F.relu(self.bn4(self.lin4(x)))
        x1 = self.lin5_1(x)
        x2 = self.lin5_2(x)
        x3 = self.lin5_3(x)
        x4 = self.lin5_4(x)
        x5 = self.lin5_5(x)
        x6 = self.lin5_6(x)
        x7 = self.lin5_7(x)
        x8 = self.lin5_8(x)
        x9 = self.lin5_9(x)
        x10 = self.lin5_10(x)
        x11 = self.lin5_11(x)
        return x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11


class DivaCNN(nn.Module):
    def __init__(self):
        super(DivaCNN, self).__init__()
        self.conv1 = nn.Conv2d(1, 16, 3)  # dilation is not added here
        self.bn1 = nn.BatchNorm2d(16)
        self.pool1 = nn.MaxPool2d(2, stride=2)
        self.drp = nn.Dropout2d(0.25)
        self.conv2 = nn.Conv2d(16, 32, 3)
        self.bn2 = nn.BatchNorm2d(32)
        self.conv3 = nn.Conv2d(32, 64, 3)
        self.bn3 = nn.BatchNorm2d(64)
        self.conv4 = nn.Conv2d(64, 128, 3)
        self.bn4 = nn.BatchNorm2d(128)
        self.conv5 = nn.Conv2d(128, 64, 3)
        self.bn5 = nn.BatchNorm2d(64)
        self.pool2 = nn.MaxPool2d(4, stride=4)
        self.fc1 = nn.Linear(64*6*2, 64)

        self.fc2_1 = nn.Linear(64, 3, bias=True)
        self.fc2_2 = nn.Linear(64, 3, bias=True)
        self.fc2_3 = nn.Linear(64, 3, bias=True)
        self.fc2_4 = nn.Linear(64, 3, bias=True)
        self.fc2_5 = nn.Linear(64, 3, bias=True)
        self.fc2_6 = nn.Linear(64, 3, bias=True)
        self.fc2_7 = nn.Linear(64, 3, bias=True)
        self.fc2_8 = nn.Linear(64, 3, bias=True)
        self.fc2_9 = nn.Linear(64, 3, bias=True)
        self.fc2_10 = nn.Linear(64, 3, bias=True)
        self.fc2_11 = nn.Linear(64, 3, bias=True)

    def forward(self, x):
        x = self.drp(self.pool1(F.relu(self.bn1(self.conv1(x)))))
        x = self.drp(F.relu(self.bn2(self.conv2(x))))
        x = self.drp(self.pool1(F.relu(self.bn3(self.conv3(x)))))
        x = self.drp(F.relu(self.bn4(self.conv4(x))))
        x = self.drp(self.pool2(F.relu(self.bn5(self.conv5(x)))))
        x = x.view(-1, 64*6*2)
        x = self.fc1(x)

        x1 = self.fc2_1(x)
        x2 = self.fc2_2(x)
        x3 = self.fc2_3(x)
        x4 = self.fc2_4(x)
        x5 = self.fc2_5(x)
        x6 = self.fc2_6(x)
        x7 = self.fc2_7(x)
        x8 = self.fc2_8(x)
        x9 = self.fc2_9(x)
        x10 = self.fc2_10(x)
        x11 = self.fc2_11(x)

        return x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11


class DivaAutoEncoder(nn.Module):
    def __init__(self):
        super(DivaAutoEncoder, self).__init__()
        self.encoder = nn.Sequential(
            nn.Conv2d(1, 16, 3, stride=3, padding=1),  # b, 16, 22, 22
            nn.ReLU(True),
            nn.MaxPool2d(2, stride=2),  # b, 16, 11, 11
            nn.Conv2d(16, 8, 3, stride=2, padding=1),  # b, 8, 6, 6
            nn.ReLU(True),
            nn.MaxPool2d(2, stride=1),  # b, 8, 5, 5
            nn.Conv2d(8, 4, 3, stride=2, padding=1),  # b, 4, 3, 3
            nn.MaxPool2d(2, stride=1),  # b, 4, 2, 2
        )
        self.decoder = nn.Sequential(
            nn.ConvTranspose2d(4, 8, 3, stride=2),  # b, 8, 5, 5
            nn.ReLU(True),
            nn.ConvTranspose2d(8, 16, 3, stride=2),  # b, 16, 11, 11
            nn.ReLU(True),
            nn.ConvTranspose2d(16, 8, 5, stride=3, padding=1),  # b, 8, 33, 33
            nn.ReLU(True),
            nn.ConvTranspose2d(8, 1, 2, stride=2, padding=1),  # b, 1, 64, 64
            nn.Tanh()
        )

    def forward(self, x):
        x = self.encoder(x)
        x = self.decoder(x)
        return x

    def encode(self, x):
        return self.encoder(x)

    def decode(self, x):
        return self.decode(x)


def train(device, train_set, val_set, num_epochs, batch_size, net, writer_path):
    train_loader = DataLoader(train_set, batch_size, shuffle=True)
    val_loader = DataLoader(val_set, len(val_set), shuffle=True)

    net.to(device)

    optimizer = optim.Adam(net.parameters())
    criterion = nn.CrossEntropyLoss()
    writer = SummaryWriter(writer_path)

    for epoch in trange(num_epochs):
        epoch_loss = 0
        for i, (inputs, labels, _) in enumerate(train_loader):
            inputs = inputs.to(device)
            labels = labels.to(device)

            optimizer.zero_grad()

            out = net(inputs)

            loss = []
            for j in range(11):
                loss.append(criterion(out[j], labels[:, j]))
            loss = sum(loss)

            epoch_loss += loss

            loss.backward()
            optimizer.step()

        epoch_loss /= (i + 1)
        writer.add_scalar('Loss/train', epoch_loss, epoch)

        # validate every 5 epochs
        if (epoch + 1) % 5 == 0:
            accuracies = [[] for i in range(11)]
            val_loss = []
            # NOTE: Since I set only one batch for validation, the for loop
            # is only for one iteration
            for val_inputs, val_labels in val_loader:
                val_inputs = val_inputs.to(device)
                val_labels = val_labels.to(device)

                val_out = net(val_inputs)
                for j in range(11):
                    val_loss.append(criterion(val_out[j], val_labels[:, j]))
                    pred = val_out[j].argmax(dim=1)
                    print(f'predict {float(sum(pred == 0))/val_labels.size(0)*100}% as 0')
                    corrects = (pred == val_labels[:, j])
                    accuracies[j].append(corrects.sum().float() / float(val_labels.size(0)))
                val_loss = sum(val_loss)

            writer.add_scalar('Loss/val', val_loss, epoch)
            accuracies = torch.tensor(accuracies)
            accuracies = torch.mean(accuracies, dim=1)
            for k in range(11):
                writer.add_scalar(f'Accuracy/feature_{k}', accuracies[k], epoch)
                print(f'Accuracy/feature_{k}: {accuracies[k]}')

    # writer.flush()
    writer.close()

    time_stamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
    Path('models').mkdir(parents=True, exist_ok=True)
    torch.save(net.state_dict(), f'models/{time_stamp}.pt')


def train_auto_encoder(device, train_set, val_set, num_epochs, batch_size, net, writer_path):
    train_loader = DataLoader(train_set, batch_size, shuffle=True)
    val_loader = DataLoader(val_set, len(val_set), shuffle=True)

    net.to(device)

    optimizer = optim.Adam(net.parameters())
    criterion = nn.MSELoss()
    writer = SummaryWriter(writer_path)

    for epoch in trange(num_epochs):
        epoch_loss = 0

        for i, (inputs, _, _) in enumerate(train_loader):
            inputs = inputs.to(device)

            optimizer.zero_grad()
            out = net(inputs)
            loss = criterion(out, inputs)

            epoch_loss += loss

            loss.backward()
            optimizer.step()

        epoch_loss /= (i + 1)
        print(f'epoch_loss: {epoch_loss}')
        writer.add_scalar('Loss/train', epoch_loss, epoch)

        # validate every 5 epochs
        if (epoch + 1) % 5 == 0:
            # NOTE: Since I set only one batch for validation, the for loop
            # is only for one iteration
            for val_inputs, _ in val_loader:
                val_inputs = val_inputs.to(device)

                val_out = net(val_inputs)
                val_loss = criterion(val_inputs, val_out)

            writer.add_scalar('Loss/val', val_loss, epoch)

    # writer.flush()
    writer.close()

    time_stamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
    Path('models').mkdir(parents=True, exist_ok=True)
    torch.save(net.state_dict(), f'models/auto-encoder-{time_stamp}.pt')


if __name__ == '__main__':
    torch.manual_seed(0)
    np.random.seed(0)

    device = torch.device("cuda:1" if torch.cuda.is_available() else "cpu")
    num_epochs = 200
    batch_size = 10

    lengths = [2795, 350, 349]

    model_name = sys.argv[1]

    if model_name == 'dnn':
        dataset = MelDataset(flat=True)
        train_set, val_set, test_set = torch.utils.data.random_split(dataset, lengths)
        dnn = DivaDNN()
        train(device, train_set, val_set, num_epochs, batch_size, dnn, 'runs/dnn')

    elif model_name == 'cnn':
        dataset = MelDataset(flat=False)
        train_set, val_set, test_set = torch.utils.data.random_split(dataset, lengths)
        cnn = DivaCNN()
        train(device, train_set, val_set, num_epochs, batch_size, cnn, 'runs/cnn')

    elif model_name == 'ae':
        dataset = MelDataset(flat=False)
        train_set, val_set, test_set = torch.utils.data.random_split(dataset, lengths)
        auto_encoder = DivaAutoEncoder()
        train_auto_encoder(device, train_set, val_set, num_epochs, batch_size, auto_encoder, 'runs/auto_encoder')
