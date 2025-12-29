import pytest
import time
from trading_bot import trading_core

def test_market_manager_concurrency():
    # Arrange
    manager = trading_core.MarketManager(4)
    symbols = ["BTC/USDT", "ETH/USDT"]
    
    # Act
    for _ in range(100):
        for s in symbols:
            manager.update_tick(s, 50000.0)
    
    time.sleep(0.1) # Breve pausa para los hilos
    
    # Assert
    assert manager.get_last_price("BTC/USDT") == 50000.0
    assert manager.get_last_price("ETH/USDT") == 50000.0