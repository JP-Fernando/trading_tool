import asyncio
import ccxt.pro as ccxtpro
import pandas as pd
import numpy as np
import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from trading_bot import trading_core
from trading_bot.engine import TradingEngine

async def run_realtime_monitor():
    engine = TradingEngine()
    exchange = ccxtpro.binance({'enableRateLimit': True})
    symbol = 'BTC/USDT'
    timeframe = '1m'
    
    # Parámetros de los indicadores
    bb_window, bb_k = 20, 2.0
    macd_f, macd_s, macd_sig = 12, 26, 9
    rsi_w = 14

    print(f"--- [INIT] Cargando historial para {symbol} ---")
    df = engine.fetch_data(timeframe=timeframe, limit=200) # Necesitamos más datos para el MACD
    
    last_processed_price = None

    try:
        print(f"{'PRICE':^10} | {'RSI':^6} | {'MACD':^7} | {'SIGNAL':^7} | {'BB_UP':^9} | {'BB_LOW':^9}")
        print("-" * 70)

        while True:
            ohlcv = await exchange.watch_ohlcv(symbol, timeframe)
            if not ohlcv: continue

            new_candle = ohlcv[-1]
            ts, o, h, l, c, v = new_candle

            if c != last_processed_price:
                # Actualizar DF
                if df.iloc[-1]['timestamp'] == ts:
                    df.iloc[-1, 1:] = [o, h, l, c, v]
                else:
                    new_row = pd.DataFrame([new_candle], columns=df.columns)
                    df = pd.concat([df, new_row], ignore_index=True).tail(200)

                # Preparar datos para C++
                close_prices = np.ascontiguousarray(df['close'].values, dtype=np.float64)

                # --- LLAMADAS AL CORE C++ ---
                
                # RSI
                rsi_v = trading_core.calculate_rsi(close_prices, rsi_w)
                
                # MACD (Recibe tupla <array, array>)
                macd_line, signal_line = trading_core.calculate_macd(close_prices, macd_f, macd_s, macd_sig)
                
                # Bollinger (Recibe tupla <array, array, array>)
                bb_up, bb_mid, bb_low = trading_core.calculate_bollinger_bands(close_prices, bb_window, bb_k)

                # Formatear salida (Tomamos el índice -1 de cada resultado de C++)
                out = (
                    f"{c:>10.2f} | {rsi_v[-1]:>6.2f} | "
                    f"{macd_line[-1]:>7.2f} | {signal_line[-1]:>7.2f} | "
                    f"{bb_up[-1]:>9.2f} | {bb_low[-1]:>9.2f}"
                )
                print(out, end='\r')
                
                last_processed_price = c

    except Exception as e:
        print(f"\n[!] Error: {e}")
    finally:
        await exchange.close()

if __name__ == "__main__":
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(run_realtime_monitor())
    except KeyboardInterrupt:
        tasks = asyncio.all_tasks(loop)
        for t in tasks: t.cancel()
        loop.run_until_complete(asyncio.gather(*tasks, return_exceptions=True))
    finally:
        loop.close()