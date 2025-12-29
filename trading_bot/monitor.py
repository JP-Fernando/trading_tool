import asyncio
import ccxt.pro as ccxtpro
import sys
import os

# Ensure local imports
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from trading_bot import trading_core

async def symbol_loop(exchange, symbol, manager):
    """
    Listens to ticks and dispatches them to MarketManager.
    """
    timeframe = '1m'
    print(f"[LOOP] Started monitoring {symbol}")
    
    try:
        # Warm-up
        # Download last 100 candles
        print(f"--- [WARM-UP] Fetching history for {symbol} ---")
        history = await exchange.fetch_ohlcv(symbol, timeframe, limit=100)
        for candle in history:
            # Update C++ buffer
            manager.update_tick(symbol, candle[4])
        
        # Start stream loop
        print(f"[LOOP] Starting real-time monitor for {symbol}")
        while True:
            # Get update through WebSocket
            ohlcv = await exchange.watch_ohlcv(symbol, timeframe)
            if not ohlcv:
                continue
            
            # Extract last closing price of current candle
            last_close = ohlcv[-1][4]
            
            # Dispatch data to C++ engine.
            manager.update_tick(symbol, last_close)
            
    except Exception as e:
        print(f"[!] Error in {symbol} loop: {e}")

async def run_realtime_monitor():
    # 1. Create MarketManager instance with 4 C++ threads
    manager = trading_core.MarketManager(4)
    
    # 2. Configure Exchange
    exchange = ccxtpro.binance({'enableRateLimit': True})
    
    # 3. List of symbols (coins) to monitor
    symbols = [
        'BTC/USDT', 
        'ETH/USDT', 
        'SOL/USDT', 
        'ADA/USDT', 
        'DOT/USDT', 
        'BNB/USDT'
        ]
    
    print(f"--- [INIT] Dispatching {len(symbols)} symbols to C++ Core ---")
    
    # 4. Create a task for each symbol
    tasks = [symbol_loop(exchange, symbol, manager) for symbol in symbols]
    
    try:
        # Concurrently run all loops
        await asyncio.gather(*tasks)
    except asyncio.CancelledError:
        pass
    finally:
        await exchange.close()

if __name__ == "__main__":
    try:
        asyncio.run(run_realtime_monitor())
    except KeyboardInterrupt:
        print("\n[OK] Stopping monitor...")