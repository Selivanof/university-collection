import os
import numpy as np
import glob
import matplotlib.pyplot as plt
import scipy.stats as stats
import pandas as pd

def get_recording_data(directory_path):
    # Find all *_recording.csv files in the specified directory
    csv_files = glob.glob(os.path.join(directory_path, '*_records.csv'))
    # Initialize lists to hold timestamps and delays
    timestamps = []
    delays = []

    # Process each CSV file
    for csv_file in csv_files:
        # Check if the file is empty
        if os.stat(csv_file).st_size == 0:
            continue  # Skip empty files

        # Read the CSV file into a DataFrame
        trade_df = pd.read_csv(csv_file)

        # Check if the "Write Delay" column exists
        if 'Write Delay' in trade_df.columns and 'Trade Timestamp' in trade_df.columns:
            # Append the Timestamp and Write Delay columns to their respective lists
            timestamps.extend(trade_df['Trade Timestamp'].to_numpy())
            delays.extend(trade_df['Write Delay'].to_numpy())
        else:
            print(f"Columns not found in {csv_file}: Skipping this file.")

    # Convert the lists of values to NumPy arrays
    return np.array(timestamps), np.array(delays)

def calculate_statistics(values):
    # Calculate statistics
    average = np.mean(values)
    median = np.median(values)
    high_1_percentile = np.percentile(values, 99)
    max_value = np.max(values)
    min_value = np.min(values)

    return average, median, high_1_percentile, max_value, min_value

def plot_combined_histogram(overall_values, first_half_values, second_half_values):
    plt.figure(figsize=(10, 6))
    
    # Determine reasonable bin count
    num_bins = 20  # Adjust this number if necessary

    # Plot histograms with normalized y-axis
    plt.hist(overall_values, bins=num_bins, range=(0, 1000), edgecolor='black', alpha=0.5, density=True, label='Total Distribution')
    plt.hist(first_half_values, bins=num_bins, range=(0, 1000), edgecolor='blue', alpha=0.7, density=True, label='First 24-Hour Distribution')
    plt.hist(second_half_values, bins=num_bins, range=(0, 1000), edgecolor='red', alpha=0.7, density=True, label='Second 24-Hour Distribution')

    # Set labels and title
    plt.title('Delay Distribution: Total, Hours 0-24, and Hours 24-48')
    plt.xlabel('Delay')
    plt.ylabel('Probability Density')
    
    # Adjust x-axis limits based on the range of the data
    plt.xlim(0, 1000)
    
    # Show grid and legend
    plt.grid(True)
    plt.legend()
    plt.show()

def plot_qq_lognormal(values, label):
    # Log-transform the values
    log_values = np.log(values[values > 0])  # Only log-transform positive values

    # Q-Q Plot
    plt.figure(figsize=(12, 6))
    stats.probplot(log_values, dist="norm", plot=plt)
    plt.title(f'Q-Q Plot of Log-Transformed Delays: {label}')
    plt.grid(True)
    plt.show()


def calculate_delay_statistics(path, half_time):
    timestamps, delays = get_recording_data(path)

    # Calculate statistics for all delays
    overall_stats = calculate_statistics(delays)

    # Print overall statistics
    print("Overall Statistics:")
    print(f"Average: {overall_stats[0]}")
    print(f"Median: {overall_stats[1]}")
    print(f"1% High (99th percentile): {overall_stats[2]}")
    print(f"Max: {overall_stats[3]}")
    print(f"Min: {overall_stats[4]}")

    # Calculate statistics for delays up to HALF_TIME
    mask_half_time = timestamps <= half_time
    delays_up_to_half_time = delays[mask_half_time]

    if delays_up_to_half_time.size > 0:
        stats_half_time = calculate_statistics(delays_up_to_half_time)
        print("\nStatistics for hours 0-24:")
        print(f"Average: {stats_half_time[0]}")
        print(f"Median: {stats_half_time[1]}")
        print(f"1% High (99th percentile): {stats_half_time[2]}")
        print(f"Max: {stats_half_time[3]}")
        print(f"Min: {stats_half_time[4]}")
    else:
        print("No delays found up to HALF_TIME.")

    # Calculate statistics for delays after HALF_TIME
    delays_after_half_time = delays[~mask_half_time]

    if delays_after_half_time.size > 0:
        stats_after_half_time = calculate_statistics(delays_after_half_time)
        print("\nStatistics for hours 24-48:")
        print(f"Average: {stats_after_half_time[0]}")
        print(f"Median: {stats_after_half_time[1]}")
        print(f"1% High (99th percentile): {stats_after_half_time[2]}")
        print(f"Max: {stats_after_half_time[3]}")
        print(f"Min: {stats_after_half_time[4]}")
        
        # Check log-normality for delays after HALF_TIME
    else:
        print("No delays found after HALF_TIME.")

    # Plot combined histogram
    plot_combined_histogram(delays, delays_up_to_half_time, delays_after_half_time)
    plot_qq_lognormal(delays, 'Hours 0-48')
    plot_qq_lognormal(delays_up_to_half_time, 'Hours 0-24')
    plot_qq_lognormal(delays_after_half_time, 'Hours 24-48')


if __name__ == "__main__":
    # Set the directory where your *_recording.csv files are located
    directory_path = "../build/csv"
    
    # Define HALF_TIME (modify according to your context)
    HALF_TIME = 1728075600000000  # This is just an example; set this to your desired value
    
    calculate_delay_statistics(directory_path, HALF_TIME)