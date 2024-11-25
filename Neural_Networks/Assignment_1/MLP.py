import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import datasets, transforms
import torchvision.transforms.functional as F
from torch.utils.data import DataLoader
import os
from tqdm import tqdm  # For progress bars

# Parameters
learning_rate = 0.001
batch_size = 32
num_epochs = 30
image_height = 32
image_width = 32
layer_neurons = [512,256,128]
color = True

def load_data(train_dir, test_dir, batch_size):
    # BW
    transform_list = [
        transforms.Resize((image_height, image_width)), 
        transforms.ToTensor(),
        transforms.Lambda(lambda img: F.adjust_brightness(img, 1.5))
    ]

    if color:
        transform_list.append(transforms.Normalize((0.3403, 0.3121, 0.3214), (0.2724, 0.2608, 0.2669)))
    else:
        transform_list.insert(0, transforms.Grayscale(num_output_channels=1))
        transform_list.append(transforms.Normalize((0.3192), (0.2584)))

    transform = transforms.Compose(transform_list)

    # Load datasets
    train_dataset = datasets.ImageFolder(root=train_dir, transform=transform)
    test_dataset = datasets.ImageFolder(root=test_dir, transform=transform)

    # Data loaders
    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
    test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

    return train_loader, test_loader, len(train_dataset.classes)

class MLPClassifier(nn.Module):
    def __init__(self, input_size, hidden_neurons, num_classes):
        super(MLPClassifier, self).__init__()
        layers = []
        layer_in = input_size

        # Hidden layers
        for neurons in hidden_neurons:
            layers.append(nn.Linear(layer_in, neurons))
            layers.append(nn.ReLU())
            layer_in = neurons

        # Output layer
        layers.append(nn.Linear(layer_in, num_classes))
        self.model = nn.Sequential(*layers)

    def forward(self, x):
        x = self.model(x)
        return x

def train_model(model, train_loader, criterion, optimizer, device):
    model.train()
    for epoch in range(num_epochs):
        total_loss = 0

        print(f'\nEpoch {epoch + 1}/{num_epochs}')
        progress_bar = tqdm(enumerate(train_loader), total=len(train_loader), desc="Training")
        
        for batch_idx, (images, labels) in progress_bar:
            images, labels = images.to(device), labels.to(device)

            # Flatten the images
            images = images.view(images.size(0), -1)

            # Forward pass
            outputs = model(images)
            loss = criterion(outputs, labels)

            # Backward pass
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

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

            # Flatten the images
            images = images.view(images.size(0), -1)

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

    # Input Size
    if(color):
        input_size = image_height * image_width * 3
    else:
        input_size = image_height * image_width
    #Model Creation
    model = MLPClassifier(input_size, layer_neurons, num_classes).to(device)

    # Loss and optimizer
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)

    # Train and Evaluate
    train_model(model, train_loader, criterion, optimizer, device)
    evaluate_model(model, test_loader, device)

if __name__ == '__main__':
    main()
