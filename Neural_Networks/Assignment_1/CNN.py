import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import datasets, transforms
import torchvision.transforms.functional as F
from torch.utils.data import DataLoader
from tqdm import tqdm

# Parameters
max_learning_rate = 0.001
base_learning_rate = 1e-5
batch_size = 32
num_epochs = 10

image_height = 32
image_width = 32
color = True

num_filters = [32, 64, 128, 256]  
kernel_size = 3
padding = 1
pooling_kernel_size = 2
pooling_stride = 1

fc_neurons = [512, 256]  

dropout_rate = 0.5

def load_data(train_dir, test_dir, batch_size):
    train_transform = transforms.Compose([
        transforms.Resize((image_height, image_width)),
        transforms.RandomRotation(30),
        transforms.ColorJitter(brightness=0.1, contrast=0.2, saturation=0.1, hue=0.1),
        transforms.ToTensor(),
        transforms.Normalize((0.3403, 0.3121, 0.3214), (0.2724, 0.2608, 0.2669)) if color else transforms.Normalize((0.3192), (0.2584))
    ])

    test_transform = transforms.Compose([
        transforms.Resize((image_height, image_width)),
        transforms.ToTensor(),
        transforms.Normalize((0.3403, 0.3121, 0.3214), (0.2724, 0.2608, 0.2669)) if color else transforms.Normalize((0.3192), (0.2584))
    ])

    # Load datasets
    train_dataset = datasets.ImageFolder(root=train_dir, transform=train_transform)
    test_dataset = datasets.ImageFolder(root=test_dir, transform=test_transform)

    # Data loaders
    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
    test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

    return train_loader, test_loader, len(train_dataset.classes)

class CNNClassifier(nn.Module):
    def __init__(self, input_channels, num_filters, kernel_size, fc_neurons, num_classes):
        super(CNNClassifier, self).__init__()
        layers = []
        in_channels = input_channels

        # Convolutional layers
        for out_channels in num_filters:
            layers.append(nn.Conv2d(in_channels, out_channels, kernel_size=kernel_size, padding=padding))
            layers.append(nn.ReLU())
            layers.append(nn.BatchNorm2d(out_channels))
            layers.append(nn.MaxPool2d(kernel_size=pooling_kernel_size, stride=pooling_stride))
            in_channels = out_channels

        self.conv = nn.Sequential(*layers)

        # Compute the flattened size after convolution
        dummy_input = torch.zeros(1, input_channels, image_height, image_width)
        conv_output_size = self.conv(dummy_input).view(1, -1).size(1)

        # Fully connected layers with Dropout
        fc_layers = []
        fc_input = conv_output_size
        for neurons in fc_neurons:
            fc_layers.append(nn.Linear(fc_input, neurons))
            fc_layers.append(nn.ReLU())
            fc_layers.append(nn.Dropout(dropout_rate))
            fc_input = neurons

        fc_layers.append(nn.Linear(fc_input, num_classes))
        self.fc = nn.Sequential(*fc_layers)

    def forward(self, x):
        x = self.conv(x)
        x = x.view(x.size(0), -1)
        x = self.fc(x)
        return x

def train_model(model, train_loader, criterion, optimizer, scheduler, device):
    model.train()
    for epoch in range(num_epochs):
        total_loss = 0

        print(f'\nEpoch {epoch + 1}/{num_epochs}')
        progress_bar = tqdm(enumerate(train_loader), total=len(train_loader), desc="Training")
        
        for batch_idx, (images, labels) in progress_bar:
            images, labels = images.to(device), labels.to(device)

            # Forward pass
            outputs = model(images)
            loss = criterion(outputs, labels)

            # Backward pass
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            scheduler.step()

            total_loss += loss.item()
            progress_bar.set_postfix(loss=loss.item())

        print(f'Epoch {epoch + 1} Loss: {total_loss / len(train_loader):.4f}')

def evaluate_model(model, test_loader, device):
    model.eval()
    correct = 0
    total = 0
    print("\nEvaluating...")
    progress_bar = tqdm(enumerate(test_loader), total=len(test_loader), desc="Evaluating")
    
    with torch.no_grad():
        for batch_idx, (images, labels) in progress_bar:
            images, labels = images.to(device), labels.to(device)

            # Forward pass
            outputs = model(images)
            _, predicted = torch.max(outputs.data, 1)
            total += labels.size(0)
            correct += (predicted == labels).sum().item()

            progress_bar.set_postfix(correct=correct, total=total, accuracy=(100 * correct / total))

    print(f'\nAccuracy: {100 * correct / total:.2f}%')

def main():
    # Paths
    train_dir = '/home/gseli/Documents/neural_nets/dataset/archive/Train'
    test_dir = '/home/gseli/Documents/neural_nets/dataset/archive/TestSeperated'

    # Device configuration
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

    # Load data
    train_loader, test_loader, num_classes = load_data(train_dir, test_dir, batch_size)

    # Input Channels
    input_channels = 3 if color else 1

    # Model Creation
    model = CNNClassifier(input_channels, num_filters, kernel_size, fc_neurons, num_classes).to(device)

    # Loss and optimizer
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=max_learning_rate, weight_decay=1e-4)

    # Learning rate scheduler
    scheduler = optim.lr_scheduler.CyclicLR(optimizer, base_lr=base_learning_rate, max_lr=max_learning_rate, step_size_up=2000, mode='triangular')

    # Train and Evaluate
    train_model(model, train_loader, criterion, optimizer, scheduler, device)
    evaluate_model(model, test_loader, device)

if __name__ == '__main__':
    main()
