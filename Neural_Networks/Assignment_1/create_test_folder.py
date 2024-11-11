import os
import shutil
import pandas as pd

csv_file_path = '/home/gseli/Documents/Neural Networks/Dataset/Test.csv'  
train_dir = '/home/gseli/Documents/Neural Networks/Dataset'  
new_train_dir = '/home/gseli/Documents/Neural Networks/Dataset/TestSeperated' 

if not os.path.exists(new_train_dir):
    os.makedirs(new_train_dir)

df = pd.read_csv(csv_file_path)

for index, row in df.iterrows():
    class_id = row['ClassId']
    img_path = os.path.join(train_dir, row['Path'])
    
    class_dir = os.path.join(new_train_dir, str(class_id))
    if not os.path.exists(class_dir):
        os.makedirs(class_dir)
    
    if os.path.exists(img_path):
        img_name = os.path.basename(img_path)
        dest_path = os.path.join(class_dir, img_name)
        
        shutil.copy2(img_path, dest_path) 
        print(f"Moved {img_name} to {class_dir}")
    else:
        print(f"Warning: Image not found - {img_path}")

print("Image organization complete.")