import ccxt
import pandas as pd
import ta

class TradingEngine:
    def __init__(self, symbol='BTC/USDT'):
        self.exchange = ccxt.binance()
        self.symbol = symbol

    def fetch_data(self, timeframe='1h', limit=100):
        """Fetches historical OHLCV data."""
        bars = self.exchange.fetch_ohlcv(self.symbol, timeframe=timeframe, limit=limit)
        df = pd.DataFrame(bars, columns=['timestamp', 'open', 'high', 'low', 'close', 'volume'])
        df['timestamp'] = pd.to_datetime(df['timestamp'], unit='ms')
        return df

    def add_indicators(self, df, sma_window=20):
        """Calcula la SMA con una ventana personalizable."""
        df[f'SMA_{sma_window}'] = df['close'].rolling(window=sma_window).mean()
        df['RSI'] = ta.momentum.RSIIndicator(df['close'], window=rsi_window).rsi()
        return df

    def get_signals(self, df, sma_window=20):
        """Genera señales basadas en la ventana elegida."""
        col_name = f'SMA_{sma_window}'
        df['Signal'] = 0
        df.loc[(df['close'] > df[col_name]) & (df['RSI'] < rsi_oversold), 'Signal'] = 1 # Compra
        df.loc[(df['close'] < df[col_name]) & (df['RSI'] > rsi_overbought), 'Signal'] = -1 # Venta
        return df
    
    def run_backtest(self, df, initial_balance=1000.0):
        """
        Simulates trading based on signals and calculates returns.
        """
        # 1. Ensure we have signals
        if 'Signal' not in df.columns:
            df = self.get_signals(df)

        # 2. Calculate daily returns (percentage change of price)
        df['market_returns'] = df['close'].pct_change()

        # 3. Strategy returns: 
        # We multiply the market return by our position from the PREVIOUS period
        # (Since we can only profit from a move AFTER we buy)
        df['strategy_returns'] = df['market_returns'] * df['Signal'].shift(1)

        # 4. Calculate cumulative returns
        df['cum_market_returns'] = (1 + df['market_returns']).cumprod()
        df['cum_strategy_returns'] = (1 + df['strategy_returns']).cumprod()

        # Cálculo del Drawdown
        # Calculamos el máximo acumulado de la curva de beneficios
        df['cum_max'] = df['cum_strategy_returns'].cummax()
        # El drawdown es la caída desde ese máximo
        df['drawdown'] = (df['cum_strategy_returns'] / df['cum_max']) - 1

        max_drawdown = df['drawdown'].min() * 100
        print(f"Max Drawdown: {max_drawdown:.2f}%")

        # 5. Calculate final balance
        final_balance = initial_balance * df['cum_strategy_returns'].iloc[-1]
        total_return_pct = (df['cum_strategy_returns'].iloc[-1] - 1) * 100

        return {
            'initial_balance': initial_balance,
            'final_balance': round(final_balance, 2),
            'total_return_pct': round(total_return_pct, 2),
            'df': df
        }