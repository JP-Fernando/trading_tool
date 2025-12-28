import pytest
import time
import numpy as np
import os
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from trading_bot import trading_core

def test_market_manager_stress():
    """
    Verifica que el MarketManager puede procesar ráfagas de datos 
    sin colapsar y que el Thread Pool responde.
    """
    num_threads = 4
    manager = trading_core.MarketManager(num_threads)
    symbols = [f"TICKER_{i}" for i in range(10)]
    
    try:
        # Enviamos 500 ticks por cada uno de los 10 símbolos
        for _ in range(500):
            for symbol in symbols:
                price = 100.0 + np.random.normal(0, 1)
                manager.update_tick(symbol, price)
        
        # Damos un margen para que los hilos terminen
        time.sleep(1)
        
        # Si llegamos aquí sin crashes (SegFaults), el test pasa
        assert True 
    except Exception as e:
        pytest.fail(f"MarketManager crashed during stress test: {e}")