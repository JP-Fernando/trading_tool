import pytest
from trading_bot.trading_tool import backtest as bt
from trading_bot.trading_tool import events as ev


@pytest.fixture
def simple_engine():
    """Fixture to initialize a basic engine with zero slippage."""
    queue = bt.EventQueue()

    no_slippage = lambda info: info.mid_price

    exec_engine = bt.ExecutionEngine(queue, no_slippage)
    backtest_engine = bt.BacktestEngine(queue, exec_engine)

    return backtest_engine, exec_engine, queue


def test_market_order_generates_fill(simple_engine):
    """
    Test the full flow: Tick -> Order -> Fill.
    Ensures that a market order is filled using the last available tick data.
    """
    engine, exec_engine, queue = simple_engine

    tick = ev.TickEvent(
        ev.make_timestamp(1000),
        "ETH",
        2000.0,
        2002.0,
        10.0,
        10.0,
        2001.0,
        1.0,
    )
    engine.push_event(tick)

    order = ev.OrderEvent(
        1,
        ev.make_timestamp(2000),
        "ETH",
        ev.Side.BUY,
        1.0,
        0.0,
        ev.OrderStatus.SUBMITTED,
        "test_strategy",
    )
    engine.push_event(order)

    engine.run()

    fills = exec_engine.get_fills()

    assert len(fills) == 1
    fill = fills[0]

    assert fill.order_id == 1
    assert fill.symbol == "ETH"
    assert fill.side == ev.Side.BUY
    assert fill.filled_quantity == 1.0

    assert fill.fill_price == 2001.0