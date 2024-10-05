import pandas as pd
import os
from datetime import datetime, timedelta

def get_timestamp_range(csv_files):
    min_timestamp = None
    max_timestamp = None

    for file in csv_files:
        # Read the CSV file, assuming the first column is the Unix timestamp in microseconds
        df = pd.read_csv(file)

        # Convert the first column (Unix timestamp in microseconds) to datetime
        df['timestamp'] = pd.to_datetime(df.iloc[:, 0], unit='us')

        # Find the min and max timestamps in the current file
        file_min = df['timestamp'].min()
        file_max = df['timestamp'].max()

        # Update the global min and max timestamps
        if min_timestamp is None or file_min < min_timestamp:
            min_timestamp = file_min
        if max_timestamp is None or file_max > max_timestamp:
            max_timestamp = file_max

    return min_timestamp, max_timestamp

def calculate_duration(start_time, end_time):
    duration = end_time - start_time
    return duration

if __name__ == "__main__":
    # Replace with the path to your directory containing the CSV files
    directory_path = "./csv/"

    # Get the list of all CSV files in the directory
    csv_files = [os.path.join(directory_path, file) for file in os.listdir(directory_path) if file.endswith('_records.csv')]

    # Get the oldest and newest timestamps across all files
    min_timestamp, max_timestamp = get_timestamp_range(csv_files)

    if min_timestamp and max_timestamp:
        # Calculate the duration between the timestamps
        duration = calculate_duration(min_timestamp, max_timestamp)
        print(f"Oldest Timestamp: {min_timestamp}")
        print(f"Newest Timestamp: {max_timestamp}")
        print(f"Duration: {duration}")
    else:
        print("No timestamps found.")