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
# Classifier Parameters
classifier_k = 3

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
    batch_size=batch_size,
    class_mode='sparse',
    shuffle=False
)

print(f"Loading testing set.")


test_generator = test_datagen.flow_from_directory(
    test_dir,
    target_size=(img_height, img_width),
    batch_size=batch_size,
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
#   kNN Implementation Functions
#===================================

def get_l2_distances(test_image, train_images):
    distances = tf.sqrt(tf.reduce_sum(tf.square(train_images - test_image), axis=(1, 2, 3)))
    return distances

def knn_predict(test_image, train_features, train_labels, k):
    """Predict the label for a single test image using 3-NN."""
    distances = get_l2_distances(test_image, train_features)
    top_k_indices = tf.argsort(distances)[:k]
    top_k_labels = tf.gather(train_labels, top_k_indices)
    predicted_label = tf.argmax(tf.math.bincount(top_k_labels))
    return predicted_label

#===================================
#          kNN Evaluation
#===================================
def evaluate_classifier():
    test_labels, test_predictions = [], []


    total_test_samples = test_generator.samples
    current_sample = 0  

    for test_images, labels in test_generator:
        for i in range(len(test_images)):
            test_image = test_images[i:i+1] 
            true_label = labels[i] 
            predicted_label = knn_predict(test_image, train_features, train_labels, classifier_k)
            
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

print(f"Number of Nearest Neighbors (k): {classifier_k}")
print(f"Prediction Accuracy            : {classifier_acc * 100:.2f}%")
print(f"Evaluation Time                : {evaluation_time:.2f} seconds")