import pandas as pd
import numpy as np
import os

def check_nan_in_csv_files(directory):
    # Get all CSV files in the specified directory
    statistics_files = [f for f in os.listdir(directory) if f.endswith('_statistics.csv')]
    records_files = [f for f in os.listdir(directory) if f.endswith('_records.csv')]
    
    all_files = statistics_files + records_files
    
    if not all_files:
        print("No _statistics.csv or _records.csv files found.")
        return
    
    for file in all_files:
        file_path = os.path.join(directory, file)
        try:
            # Read the CSV file
            df = pd.read_csv(file_path)
            
            # Check for NaN values
            nan_rows = df[df.isnull().any(axis=1)]
            if not nan_rows.empty:
                print(f"File '{file}' has NaN values in the following rows:")
                print(nan_rows)
            else:
                print(f"File '{file}' has no NaN values.")
        
        except Exception as e:
            print(f"Error processing file '{file}': {e}")

# Specify the directory containing your CSV files
directory_path = "./csv/"
check_nan_in_csv_files(directory_path)