import time
import datetime
import pandas as pd
from engine import TradingEngine

def run_monitor(symbol='BTC/USDT', timeframe='1h', interval=60):
    # Initialize the engine
    engine = TradingEngine(symbol=symbol)
    
    # State management variables
    is_position_open = False
    entry_price = 0.0
    cumulative_pnl = 0.0
    last_signal = 0

    print(f"--- Real-Time Monitor Started: {symbol} ---")
    
    while True:
        try:
            # 1. Data acquisition and processing
            raw_df = engine.fetch_data(timeframe=timeframe, limit=100)
            df_with_indicators = engine.add_indicators(raw_df, sma_window=20, rsi_window=14)
            df_with_signals = engine.get_signals(df_with_indicators, sma_window=20)
            
            last_row = df_with_signals.iloc[-1]
            current_price = last_row['close']
            current_signal = last_row['Signal']
            timestamp = datetime.datetime.now().strftime('%H:%M:%S')

            # 2. Execution Logic (Simulation)
            if current_signal != last_signal:
                # Logic for Opening a Long Position
                if current_signal == 1 and not is_position_open:
                    entry_price = current_price
                    is_position_open = True
                    print(f"\n[{timestamp}] 🟢 BUY Order Simulated at {entry_price}")
                
                # Logic for Closing the Position
                elif current_signal == -1 and is_position_open:
                    trade_pnl = (current_price - entry_price) / entry_price
                    cumulative_pnl += trade_pnl
                    is_position_open = False
                    print(f"\n[{timestamp}] 🔴 SELL Order Simulated at {current_price}")
                    print(f"      Trade PnL: {trade_pnl*100:.2f}%")
                    print(f"      Total Acc. PnL: {cumulative_pnl*100:.2f}%")
                
                last_signal = current_signal

            # 3. Dynamic Console Output
            unrealized_pnl = 0.0
            if is_position_open:
                unrealized_pnl = (current_price - entry_price) / entry_price
            
            # Using carriage return \r to update the same line in the terminal
            status_msg = f"[{timestamp}] Price: {current_price:.2f} | Live PnL: {unrealized_pnl*100:.2f}% | Signal: {current_signal}"
            print(f"\r{status_msg}", end="", flush=True)
            # Log
            engine.log_status(current_price, current_signal, unrealized_pnl, cumulative_pnl, is_position_open)


        except Exception as e:
            print(f"\n[ERROR]: Runtime exception occurred: {e}")
        
        # Sleep for the defined interval (e.g., 60 seconds)
        time.sleep(interval)

if __name__ == "__main__":
    run_monitor()