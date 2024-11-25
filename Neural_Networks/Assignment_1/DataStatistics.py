import torch
from torchvision import datasets, transforms
from torch.utils.data import DataLoader


train_dir = '/home/gseli/Documents/neural_nets/dataset/archive/Train'


transform = transforms.Compose([
    transforms.Resize((32, 32)),  
    transforms.ToTensor()        
])

def calculate_mean_std(data_dir):

    dataset = datasets.ImageFolder(root=data_dir, transform=transform)
    loader = DataLoader(dataset, batch_size=64, shuffle=False, num_workers=2)


    channel_sum = torch.zeros(3)
    channel_squared_sum = torch.zeros(3)
    num_pixels = 0


    for images, _ in loader:
       
        batch_size, channels, height, width = images.shape
        num_pixels += batch_size * height * width

       
        channel_sum += images.sum(dim=[0, 2, 3]) 
        channel_squared_sum += (images ** 2).sum(dim=[0, 2, 3])

    mean = channel_sum / num_pixels
    std = (channel_squared_sum / num_pixels - mean ** 2).sqrt()

    return mean, std

if __name__ == "__main__":
    print("Calculating statistics for the training dataset...")
    train_mean, train_std = calculate_mean_std(train_dir)
    print(f"Train Dataset Mean: {train_mean}")
    print(f"Train Dataset Std: {train_std}")

