# This script check whether the statistic results of the Finnhub WebSocket Client are valid.
# To be valid, the client must have calulated the statistics based on ALL the trades it had received
# prior to writing the statistics to the file

import numpy as np
import pandas as pd
import os
import glob
import argparse

# ANSI escape codes for color
RED = '\033[91m'
GREEN = '\033[92m'
RESET = '\033[0m'

TRADE_TIMESTAMP_INDEX = 0
TRADE_ARRIVAL_DELAY_INDEX = 1
TRADE_RECORD_DELAY_INDEX = 2
TRADE_VOLUME_INDEX = 3
TRADE_PRICE_INDEX = 4

STATISTICS_MINUTE_INDEX = 0
STATISTICS_WRITE_DELAY_INDEX = 1
STATISTICS_OPEN_INDEX = 2
STATISTICS_CLOSE_INDEX = 3
STATISTICS_MIN_INDEX = 4
STATISTICS_MAX_INDEX = 5
STATISTICS_MINUTE_VOLUME_INDEX = 6
STATISTICS_AVERAGE_INDEX = 7
STATISTICS_TOTAL_VOLUME_INDEX = 8

def get_arrival_timestamp(trade_timestamp_mp, arrival_delay_us):
    # Convert milliseconds to seconds and add microseconds
    return trade_timestamp_mp + arrival_delay_us

def calculate_average(trades):
    trades = np.array(trades)
    if trades.size > 0:
        prices = trades[:, TRADE_PRICE_INDEX]
        volumes = trades[:, TRADE_VOLUME_INDEX]
        
        total_volume = np.sum(volumes)
        total_sum = np.sum(prices * volumes)
        
        if total_volume > 0:
            return (total_sum / total_volume), total_volume
    return 0, 0

def calculate_candlestick(trades):
    open_price = 0
    close_price = 0
    min_price = 0
    max_price = 0
    volume = 0
    if trades.size > 0:
        prices = trades[:, TRADE_PRICE_INDEX]
        volumes = trades[:, TRADE_VOLUME_INDEX]
        open_price = prices[0]
        close_price = prices[-1]
        min_price = np.min(prices)
        max_price = np.max(prices)
        volume = np.sum(volumes)
    
    return open_price, close_price, min_price, max_price, volume


def validate_statistics(records_file, statistics_file):
    # Load the CSV file without headers
    if os.stat(records_file).st_size == 0:
        trade_df = pd.DataFrame([[0, 0, 0, 0, 0]], columns=[0, 1, 2, 3, 4])     
    else:
        trade_df = pd.read_csv(records_file, header=0)
    trades = trade_df.to_numpy()
    statistics_df = pd.read_csv(statistics_file, header=0)
    statistics = statistics_df.to_numpy()


    sorted_trades = trades[np.lexsort((trades[:, TRADE_RECORD_DELAY_INDEX], 
                                trades[:, TRADE_ARRIVAL_DELAY_INDEX], 
                                trades[:, TRADE_TIMESTAMP_INDEX]))]

    correct_count = 0
    wrong_count = 0

    for statistic in statistics:
        # Define the time window for the current minute
        minute_end = statistic[STATISTICS_MINUTE_INDEX]  # Start of the minute, in microseconds
        minute_start = minute_end - 60000000  # End of the minute, in microseconds
        # Define the 15-minute window
        fifteen_minutes_ago = minute_end - 15 * 60000000

        # Define constants as NumPy arrays for vectorized operations
        trade_timestamps = sorted_trades[:, TRADE_TIMESTAMP_INDEX]
        trade_arrival_delays = sorted_trades[:, TRADE_ARRIVAL_DELAY_INDEX]
        trade_arrival = get_arrival_timestamp(trade_timestamps, trade_arrival_delays)

        write_time = minute_end + statistic[STATISTICS_WRITE_DELAY_INDEX]
        # Create boolean masks for filtering
        candlestick_mask = (minute_start <= trade_timestamps) & (trade_timestamps < minute_end) & (trade_arrival < write_time)
        moving_average_mask = (fifteen_minutes_ago <= trade_timestamps) & (trade_timestamps < minute_end) & (trade_arrival < write_time)

        # Filter trades using the masks
        candlestick_trades = sorted_trades[candlestick_mask]
        moving_average_trades = sorted_trades[moving_average_mask]

        # Calculate statistics from trades
        open_price, close_price, min_price, max_price, volume = calculate_candlestick(candlestick_trades)
        moving_average, moving_volume = calculate_average(moving_average_trades)
        
        # Compare with statistics file
        if not (np.isclose(open_price, statistic[STATISTICS_OPEN_INDEX]) and
                np.isclose(close_price, statistic[STATISTICS_CLOSE_INDEX]) and
                np.isclose(min_price, statistic[STATISTICS_MIN_INDEX]) and
                np.isclose(max_price, statistic[STATISTICS_MAX_INDEX]) and
                np.isclose(volume, statistic[STATISTICS_MINUTE_VOLUME_INDEX]) and
                np.isclose(moving_volume, statistic[STATISTICS_TOTAL_VOLUME_INDEX]) and
                np.isclose(moving_average, statistic[STATISTICS_AVERAGE_INDEX])):
            wrong_count += 1
            print(f"{RED}Discrepancy found for timestamp {statistic[STATISTICS_MINUTE_INDEX]}.{RESET}")
            print(f"{RED}Calculated - Open: {open_price}, Close: {close_price}, Min: {min_price}, Max: {max_price}, Volume: {volume}, MA: {moving_average}{RESET}, MV: {moving_volume}")
            print(f"{RED}Reported - Open: {statistic[STATISTICS_OPEN_INDEX]}, Close: {statistic[STATISTICS_CLOSE_INDEX]}, Min: {statistic[STATISTICS_MIN_INDEX]}, Max: {statistic[STATISTICS_MAX_INDEX]}, Volume: {statistic[STATISTICS_MINUTE_VOLUME_INDEX]}, MA: {statistic[STATISTICS_AVERAGE_INDEX]}{RESET},  MV: {statistic[STATISTICS_TOTAL_VOLUME_INDEX]}")
        else:
            correct_count += 1


    # Print summary with colors
    print(f"{GREEN}Matching minutes: {correct_count}{RESET}")
    print(f"{RED}Invalid minutes: {wrong_count}{RESET}")

def validate_all_csv_pairs(folder_path):
    # Use glob to find all _records.csv files
    record_files = glob.glob(os.path.join(folder_path, '*_records.csv'))
    
    # Iterate over each _records.csv file
    for record_file_path in record_files:
        base_name = os.path.basename(record_file_path).replace('_records.csv', '')
        statistics_file_path = os.path.join(folder_path, f"{base_name}_statistics.csv")
        
        # Check if the matching _statistics.csv file exists
        if os.path.exists(statistics_file_path):
            print(f"Validating pair: {os.path.basename(record_file_path)} and {os.path.basename(statistics_file_path)}")
            validate_statistics(record_file_path, statistics_file_path)
        else:
            print(f"{RED}Missing statistics file for {os.path.basename(record_file_path)}{RESET}")


if __name__ == "__main__":
    # Use argparse to get the folder path from the command line
    parser = argparse.ArgumentParser(description="Validate CSV record and statistics file pairs in a folder.")
    parser.add_argument('folder_path', type=str, help="Path to the folder containing CSV files")
    args = parser.parse_args()
    
    # Call the function with the folder path from the command line
    validate_all_csv_pairs(args.folder_path)