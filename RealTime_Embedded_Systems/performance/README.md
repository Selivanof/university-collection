<div align="center">
<h3 align="center">48-Hour Performance Measurements</h3>
</div>

The performance measurement's files were upload with a slight delay, due to Github not accepting files larger than 100MB.

As a note, during the 48-hour testing window, Finnhub often closed the connection without a reason. This is probably not a client issue as it was tested thoroughly to find any possible client-side connection closes (even as far as changing the entire client implementation from lws to websocket++).

Moreover, sometimes during reconnections Finnhub denied to recognize the BINANCE and OANDA symbols, even though it previously had no issue with their subscription messages (INVALID_SYMBOL response). This might be a free API limit issue, but there was not enough time to be tested more extensively before the deadline. They do not affect the delay measurements. A simple counter-measure would be to re-subscribe to the messages every minute, using the existing TimedWorkers.

The headers for all the recorded csv files were added with the scripts found in this repo.