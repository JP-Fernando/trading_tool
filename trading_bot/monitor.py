import asyncio
import ccxt.pro as ccxtpro
import pandas as pd
import numpy as np
import sys
import os

# 1. Ensure the root directory is in sys.path for local imports
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# 2. Project-specific imports
from trading_bot import trading_core  # Compiled C++ module
from trading_bot.engine import TradingEngine

async def run_realtime_monitor():
    """
    Optimized monitor: integrates WebSockets, C++ indicators, 
    and native decision logic with visual console output.
    """
    engine = TradingEngine()
    exchange = ccxtpro.binance({'enableRateLimit': True})
    
    symbol = 'BTC/USDT'
    timeframe = '1m'
    
    # Indicator parameters
    rsi_window = 14
    bb_window, bb_k = 20, 2.0
    macd_fast, macd_slow, macd_signal = 12, 26, 9

    print(f"--- [INIT] Loading initial history for {symbol} ---")
    # Fetch data using keyword arguments to avoid positional errors
    df = engine.fetch_data(timeframe=timeframe, limit=200)
    
    last_processed_price = None

    # ANSI Color Codes for terminal output
    GREEN = "\033[92m"
    RED = "\033[91m"
    RESET = "\033[0m"
    BOLD = "\033[1m"

    try:
        print(f"\n{BOLD}{'SYMBOL':<10} | {'PRICE':<10} | {'RSI':<6} | {'MACD':<7} | {'BB_UP':<9} | {'SIGNAL'}{RESET}")
        print("-" * 75)

        while True:
            # Async suspension: wait for the next WebSocket event
            ohlcv = await exchange.watch_ohlcv(symbol, timeframe)
            if not ohlcv:
                continue

            new_candle = ohlcv[-1] # [timestamp, open, high, low, close, volume]
            ts, o, h, l, c, v = new_candle

            # Only process if the price has changed to save CPU cycles
            if c != last_processed_price:
                
                # 1. Update DataFrame efficiently
                if df.iloc[-1]['timestamp'] == ts:
                    # Update current candle (candle in formation)
                    df.iloc[-1, 1:] = [o, h, l, c, v]
                else:
                    # New candle closed: add and trim the buffer
                    new_row = pd.DataFrame([new_candle], columns=df.columns)
                    df = pd.concat([df, new_row], ignore_index=True).tail(200)

                # 2. Prepare data for C++ (ensure contiguous memory)
                close_prices = np.ascontiguousarray(df['close'].values, dtype=np.float64)

                # 3. Calculate indicators in C++ Core
                rsi_values = trading_core.calculate_rsi(close_prices, rsi_window)
                macd_line, _ = trading_core.calculate_macd(close_prices, macd_fast, macd_slow, macd_signal)
                bb_upper, _, bb_lower = trading_core.calculate_bollinger_bands(close_prices, bb_window, bb_k)

                # 4. Run decision logic in C++ Core
                signal = trading_core.check_signals(rsi_values, close_prices, bb_upper, bb_lower)

                # 5. Format signal output
                if signal == 1:
                    sig_str = f"{GREEN}BUY{RESET}"
                elif signal == -1:
                    sig_str = f"{RED}SELL{RESET}"
                else:
                    sig_str = "WAIT"

                # 6. Dynamic Output using \r to overwrite the same line
                output = (
                    f"{symbol:<10} | {c:>10.2f} | {rsi_values[-1]:>6.1f} | "
                    f"{macd_line[-1]:>7.2f} | {bb_upper[-1]:>9.2f} | {BOLD}{sig_str}{RESET}"
                )
                
                print(output, end='\r', flush=True)
                
                last_processed_price = c

    except asyncio.CancelledError:
        pass
    except Exception as e:
        print(f"\n[!] Monitor Error: {e}")
    finally:
        print("\n--- [EXIT] Closing exchange connection ---")
        await exchange.close()

if __name__ == "__main__":
    # Event Loop configuration with clean shutdown management
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    
    try:
        loop.run_until_complete(run_realtime_monitor())
    except KeyboardInterrupt:
        # On Ctrl+C, cancel all pending tasks gracefully
        tasks = asyncio.all_tasks(loop)
        for t in tasks:
            t.cancel()
        
        loop.run_until_complete(asyncio.gather(*tasks, return_exceptions=True))
    finally:
        loop.close()
        print("[OK] Program terminated.")