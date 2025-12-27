from engine import TradingEngine
import time

def run_monitor(symbol='BTC/USDT', timeframe='1h', interval=60):
    engine = TradingEngine(symbol=symbol)
    
    while True:
        df = engine.fetch_data(timeframe=timeframe, limit=100)
        df = engine.add_indicators(df, sma_window=20)
        df = engine.get_signals(df, sma_window=20)
        last_signal = df['Signal'].iloc[-1]
        print(f"Última señal para {symbol}: {last_signal}")
        time.sleep(interval)  # Espera para la siguiente iteración

if __name__ == "__main__":
    run_monitor()
