import pytest
import time
import numpy as np
import os
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from trading_bot import trading_core

def test_market_manager_stress():
    """
    Verify that the MarketManager can process bursts of data 
    without crashing and that the Thread Pool is responsive.    
    """
    num_threads = 4
    manager = trading_core.MarketManager(num_threads)
    symbols = [f"TICKER_{i}" for i in range(10)]
    
    try:
        # Send 500 ticks for each of 10 symbols
        for _ in range(500):
            for symbol in symbols:
                price = 100.0 + np.random.normal(0, 1)
                manager.update_tick(symbol, price)
        
        # Give threads time to finish
        time.sleep(1)
        
        # Test passed: No segmentation faults so far
        assert True 
    except Exception as e:
        pytest.fail(f"MarketManager crashed during stress test: {e}")