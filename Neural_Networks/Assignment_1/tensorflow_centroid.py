import tensorflow as tf
import numpy as np
from tensorflow.keras.preprocessing.image import ImageDataGenerator
import pandas as pd
from PIL import Image
import os
import time

#===================================
#            Parameters
#===================================
# Dataset Parameters
dataset_dir = '/home/gseli/Documents/Neural Networks/Dataset/'
img_height = 30
img_width = 30
batch_size = 64 #Irrelevant for our usage with the knn classifier

#===================================
#         Dataset Importing
#===================================

# Define the subdirectories to be created
train_dir = os.path.join(dataset_dir, 'Train')
test_dir = os.path.join(dataset_dir, 'Test')

# Use tensorflow's ImageDataGenerators to import the image datasets
train_datagen = ImageDataGenerator(rescale=1.0/255.0)
test_datagen = ImageDataGenerator(rescale=1.0/255.0)

print(f"Loading training set.")

train_generator = train_datagen.flow_from_directory(
    train_dir,
    target_size=(img_height, img_width),
    batch_size=64, 
    class_mode='sparse',
    shuffle=False
)

print(f"Loading testing set.")


test_generator = test_datagen.flow_from_directory(
    test_dir,
    target_size=(img_height, img_width),
    batch_size=64,
    class_mode='sparse',
    shuffle=False
)

# Extract features and labels from the training dataset
train_features, train_labels = [], []
for images, labels in train_generator:
    train_features.append(images)
    train_labels.append(labels)
    if len(train_features) * batch_size >= train_generator.samples:
        break

train_features = np.concatenate(train_features)
train_labels = np.concatenate(train_labels)

print(f"Training set ready.")


#===================================
#   Nearest Centroid Classifier 
#===================================

def compute_centroids(train_features, train_labels, num_classes):
    """Compute the centroids for each class using TensorFlow."""
    centroids = []
    for label in range(num_classes):
        class_features = tf.boolean_mask(train_features, train_labels == label)
        centroid = tf.reduce_mean(class_features, axis=0)
        centroids.append(centroid)
    return tf.stack(centroids)

def get_l2_distances(test_image, targets):
    distances = tf.sqrt(tf.reduce_sum(tf.square(targets - test_image), axis=(1, 2, 3)))
    return distances

def nearest_centroid_predict(test_image, centroids):
    """Predict the label for a single test image using the nearest centroid approach."""
    distances = get_l2_distances(test_image, centroids)
    predicted_label = tf.argmin(distances, axis=0)
    return predicted_label



#===================================
#          Evaluation
#===================================
def evaluate_classifier():
    test_labels, test_predictions = [], []

    num_classes = len(train_generator.class_indices) 
    centroids = compute_centroids(train_features, train_labels, num_classes)

    total_test_samples = test_generator.samples
    current_sample = 0

    for test_images, labels in test_generator:
        for i in range(len(test_images)):
            test_image = test_images[i:i+1]  
            true_label = labels[i]
            predicted_label = nearest_centroid_predict(test_image, centroids)
            
            test_labels.append(true_label)
            test_predictions.append(predicted_label.numpy()) 
            
            current_sample += 1
            print(f"Progress: {current_sample} out of {total_test_samples}", end='\r')
        
        if current_sample >= total_test_samples:
            break

    print("\n Predictions Complete.")

    test_labels = np.array(test_labels)
    test_predictions = np.array(test_predictions)
    accuracy = np.mean(test_predictions == test_labels)
    return accuracy

start_time = time.time()
classifier_acc = evaluate_classifier()
end_time = time.time()
evaluation_time = end_time - start_time

print(f"Prediction Accuracy            : {classifier_acc * 100:.2f}%")
print(f"Evaluation Time                : {evaluation_time:.2f} seconds")