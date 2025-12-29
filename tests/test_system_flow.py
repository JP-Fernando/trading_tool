import pytest
import time
from trading_bot import trading_core

def test_full_signal_generation_flow():
    """
    Simulates the full monitor.py execution flow to ensure that
    the MarketManager detects signals under known conditions.
    """
    # 1. Setup
    captured_signals = []
    def test_log_callback(level, msg):
        # Only store if level is SIGNAL
        if level == trading_core.LogLevel.SIGNAL:
            captured_signals.append(msg)

    # Inject callback
    trading_core.set_log_callback(test_log_callback)

    manager = trading_core.MarketManager(num_threads=2)
    symbol = "TEST/USDT"
    
    # 2. Generate data that forces a low RSI (oversold) and
    # price below the lower Bollinger Band
    # We need at least 26 data points for MACD / Bollinger Bands
    prices = [100.0] * 50  # Initial stability
    prices.extend([95.0, 90.0, 85.0, 80.0, 70.0, 60.0])  # Sharp drop
    
    # 3. Inject data (simulating the monitor.py loop)
    for p in prices:
        manager.update_tick(symbol, p)
    
    # 4. Small delay to allow the ThreadPool to process
    timeout = 1.0
    start = time.time()
    while len(captured_signals) == 0 and (time.time() - start) < timeout:
        # while loop: active wait
        time.sleep(0.05)    

    # 5. Verification
    # Check that the engine is alive and responsive
    last_p = manager.get_last_price(symbol)
    assert last_p == 60.0
    # Check that signals were detected
    assert len(captured_signals) > 0, "No signal detected during price drop."
    # Assert last signals  
    assert "BUY" in captured_signals[-1]
    assert symbol in captured_signals[-1]   

    # 6. Cleanup
    trading_core.set_log_callback(None)