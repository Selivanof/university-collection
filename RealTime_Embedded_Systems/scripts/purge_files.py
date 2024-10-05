import os
import csv

# Define the directory where your CSV files are located
directory = "./csv"

# Threshold timestamp value
threshold_timestamp = 1728158400000000

# Loop through all files in the directory
for filename in os.listdir(directory):
    # Process files that end with *_statistics.csv or *_records.csv
    if filename.endswith("_statistics.csv") or filename.endswith("_records.csv"):
        filepath = os.path.join(directory, filename)
        filtered_rows = []

        # Read the file and filter rows based on timestamp
        with open(filepath, 'r', newline='') as csvfile:
            csvreader = csv.reader(csvfile)
            # Read the header
            header = next(csvreader)
            filtered_rows.append(header)  # Keep the header

            # Process the remaining rows
            for row in csvreader:
                # Ensure there's at least one column and check if the first column (timestamp) meets the condition
                if row and int(row[0]) <= threshold_timestamp:
                    filtered_rows.append(row)

        # Write the filtered rows back to the file
        with open(filepath, 'w', newline='') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerows(filtered_rows)

        print(f"Processed {filename}: Filtered and saved.")
