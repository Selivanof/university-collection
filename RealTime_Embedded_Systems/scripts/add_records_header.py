import os
import glob
import pandas as pd

def add_headers_to_csv_files(directory_path):
    # Find all *_recording.csv files in the specified directory
    csv_files = glob.glob(os.path.join(directory_path, '*_records.csv'))

    # Define the new header
    new_header = "Trade Timestamp,Arrival Delay,Write Delay,Volume,Price\n"

    # Process each CSV file
    for csv_file in csv_files:
        # Check if the file is empty
        if os.stat(csv_file).st_size == 0:
            print(f"Skipping empty file: {csv_file}")
            continue  # Skip empty files

        # Read the CSV file into a DataFrame
        df = pd.read_csv(csv_file, header=None)

        # Write the new header and existing data back to the file
        with open(csv_file, 'r+') as file:
            # Read the existing content
            content = file.read()
            # Move the cursor to the beginning of the file
            file.seek(0)
            # Write the new header
            file.write(new_header)
            # Write the old content back to the file
            file.write(content)

        print(f"Updated file: {csv_file}")

if __name__ == "__main__":
    # Set the directory where your *_recording.csv files are located
    directory_path = "../build/csv"

    add_headers_to_csv_files(directory_path)
