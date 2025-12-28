from engine import TradingEngine
import time
import datetime
import pandas as pd

def run_monitor(symbol='BTC/USDT', 
                timeframe='1h', 
                interval=60, 
                sma_window=20, 
                rsi_overbought=60, 
                rsi_oversold=50, 
                use_macd=False, 
                use_bbands=False,
                history_length=50):
    """
    Runs a monitoring loop that fetches data, computes indicators, and prints trading signals at regular intervals.
    
    :param symbol: Trading pair symbol
    :param timeframe: Timeframe for OHLCV data
    :param interval: Interval in seconds between data fetches
    :param sma_window: SMA window size
    :param rsi_overbought: RSI overbought threshold
    :param rsi_oversold: RSI oversold threshold
    :param use_macd: Whether to use MACD indicator
    :param use_bbands: Whether to use Bollinger Bands indicator
    :param history_length: Length of history signals
    """
    engine = TradingEngine(symbol=symbol)
    print(f"Starting monitor for {symbol} - timeframe: {timeframe}")
    history = pd.DataFrame(columns=['timestamp', 'price', 'signal', 'signal_str'])
    
    while True:
        try:
            # Fetch data
            df = engine.fetch_data(timeframe=timeframe, limit=100)
            df = engine.add_indicators(df, 
                                       sma_window=sma_window, 
                                       rsi_window=14, 
                                       ema_window=20, 
                                       bb_window=20)
            
            # Get signals
            df = engine.get_signals(df, 
                                    sma_window=sma_window, 
                                    rsi_overbought=rsi_overbought, 
                                    rsi_oversold=rsi_oversold, 
                                    use_macd=use_macd, 
                                    use_bbands=use_bbands)
            last_signal = df['Signal'].iloc[-1]
            last_close = df['close'].iloc[-1]

            # Save timestamp
            timestamp = datetime.datetime.now()
            signal_str = {1: 'BUY', -1: 'SELL', 0: 'HOLD'}.get(last_signal, 'HOLD')

            # Store
            history = pd.concat([history, 
                                 pd.DataFrame([
                                     {'timestamp': timestamp, 
                                    'price': last_close, 
                                    'signal': last_signal, 
                                    'signal_str': signal_str}])], 
                                    ignore_index=True)            
            # Keep last (history_length) signals
            history = history.tail(history_length)  

            # Print status
            print(f"[{timestamp.strftime('%Y-%m-%d %H:%M:%S')}] {symbol} - Price: {last_close:.2f} | Last Signal: {signal_str}")
        
        except Exception as e:
            print(f"Error fetching data or calculating signals: {e}")

        time.sleep(interval)  # Wait for next iteration

if __name__ == "__main__":
    run_monitor()
