import ccxt
import pandas as pd
import ta

class TradingEngine:
    def __init__(self, symbol='BTC/USDT'):
        self.exchange = ccxt.binance()
        self.symbol = symbol

    def fetch_data(self, timeframe='1h', limit=100):
        """Fetches historical OHLCV data.
        timeframe: Timeframe for OHLCV data (e.g., '1h', '15m')
        limit: Number of data points to fetch
        Returns a DataFrame with OHLCV data.
        """
        bars = self.exchange.fetch_ohlcv(self.symbol, timeframe=timeframe, limit=limit)
        df = pd.DataFrame(bars, columns=['timestamp', 'open', 'high', 'low', 'close', 'volume'])
        df['timestamp'] = pd.to_datetime(df['timestamp'], unit='ms')
        return df

    def add_indicators(self, df, sma_window=20, ema_window=None, rsi_window=None, macd_fast=12, macd_slow=26, macd_signal=9, bb_window=None):
        """Computes a number of indicators.
        df: DataFrame with OHLCV data
        sma_window: Window for Simple Moving Average
        ema_window: Window for Exponential Moving Average
        rsi_window: Window for Relative Strength Index
        macd_fast: Fast period for MACD
        macd_slow: Slow period for MACD
        macd_signal: Signal period for MACD
        bb_window: Window for Bollinger Bands
        Returns the DataFrame with new indicator columns.
        """

        # Simple Moving Average
        if sma_window:
            df[f'SMA_{sma_window}'] = df['close'].rolling(window=sma_window).mean()

        # Exponential Moving Average
        if ema_window:
            df[f'EMA_{ema_window}'] = df['close'].ewm(span=ema_window, adjust=False).mean()

        # Relative Strength Index
        if rsi_window:
            df['RSI'] = ta.momentum.RSIIndicator(df['close'], window=rsi_window).rsi()

        # Moving Average Convergence Divergence
        if macd_fast and macd_slow and macd_signal:
            macd_indicator = ta.trend.MACD(df['close'], window_fast=macd_fast, window_slow=macd_slow, window_sign=macd_signal)
            df['MACD'] = macd_indicator.macd()
            df['MACD_Signal'] = macd_indicator.macd_signal()

        # Bollinger Bands
        if bb_window:
            bb_indicator = ta.volatility.BollingerBands(df['close'], window=bb_window, window_dev=2)
            df['BB_High'] = bb_indicator.bollinger_hband()
            df['BB_Low'] = bb_indicator.bollinger_lband()

        return df

    def get_signals(self, df, sma_window=10, ema_window=None, rsi_overbought=60, rsi_oversold=50, use_macd=False, use_bbands=False):
        """Generates composite signals based on optional indicators.
        df: DataFrame with OHLCV data and indicators
        sma_window: Window for Simple Moving Average
        ema_window: Window for Exponential Moving Average
        rsi_overbought: RSI threshold for overbought condition
        rsi_oversold: RSI threshold for oversold condition
        use_macd: Whether to use MACD signals
        use_bbands: Whether to use Bollinger Bands signals
        Returns the DataFrame with a new 'Signal' column.
        1 for Buy, -1 for Sell, 0 for Hold
        """
        col_sma = f'SMA_{sma_window}' if sma_window else None
        df['Signal'] = 0  # Default to Hold

        buy_condition = pd.Series(True, index=df.index) # Buy condition starts as True
        sell_condition = pd.Series(True, index=df.index) # Sell condition starts as True

        # SMA condition
        if col_sma:
            buy_condition &= (df['close'] > df[col_sma])
            sell_condition &= (df['close'] < df[col_sma])

        # EMA condition
        if ema_window:
            col_ema = f'EMA_{ema_window}'
            buy_condition &= (df['close'] > df[col_ema])
            sell_condition &= (df['close'] < df[col_ema])

        # RSI condition
        if 'RSI' in df.columns:
            buy_condition &= (df['RSI'] < rsi_oversold)
            sell_condition &= (df['RSI'] > rsi_overbought)

        # MACD condition
        if use_macd and 'MACD' in df.columns and 'MACD_Signal' in df.columns:
            buy_condition &= (df['MACD'] > df['MACD_Signal'])
            sell_condition &= (df['MACD'] < df['MACD_Signal'])

        # Bollinger Bands condition
        if use_bbands and 'BB_Low' in df.columns and 'BB_High' in df.columns:
            buy_condition &= (df['close'] < df['BB_Low'])
            sell_condition &= (df['close'] > df['BB_High'])

        df.loc[buy_condition, 'Signal'] = 1
        df.loc[sell_condition, 'Signal'] = -1

        return df
    
    def run_backtest(self, df, initial_balance=1000.0):
        """
        Simulates trading based on signals and calculates returns.
        df: DataFrame with OHLCV data and signals
        initial_balance: Starting capital for the backtest
        Returns a dictionary with performance metrics.
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

        # Drawdown calculation
        # Compute the cumulative maximum of the strategy returns
        df['cum_max'] = df['cum_strategy_returns'].cummax()
        # Drawdown is defined as the percentage drop from the cumulative maximum
        df['drawdown'] = (df['cum_strategy_returns'] / df['cum_max']) - 1

        # 5. Calculate final balance
        final_balance = initial_balance * df['cum_strategy_returns'].iloc[-1]
        total_return_pct = (df['cum_strategy_returns'].iloc[-1] - 1) * 100

        return {
            'initial_balance': initial_balance,
            'final_balance': round(final_balance, 2),
            'total_return_pct': round(total_return_pct, 2),
            'df': df
        }