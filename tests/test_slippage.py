from trading_bot.trading_tool import backtest as bt
from trading_bot.trading_tool import events as ev


def test_custom_slippage_model_from_python():
    """
    Ensure that the C++ ExecutionEngine correctly calls
    a Python-defined lambda for slippage calculation.
    """
    queue = bt.EventQueue()

    def aggressive_slippage(info):
        base_price = info.mid_price
        return base_price + 5.0 if info.side == ev.Side.BUY else base_price - 5.0

    exec_engine = bt.ExecutionEngine(queue, aggressive_slippage)
    engine = bt.BacktestEngine(queue, exec_engine)

    tick = ev.TickEvent(
        ev.make_timestamp(1),
        "SOL",
        99.0,
        101.0,
        100.0,
        100.0,
        100.0,
        1.0,
    )
    engine.push_event(tick)

    order = ev.OrderEvent(
        99,
        ev.make_timestamp(2),
        "SOL",
        ev.Side.BUY,
        10.0,
        0.0,
        ev.OrderStatus.SUBMITTED,
        "slip_test",
    )
    engine.push_event(order)

    engine.run()

    fills = exec_engine.get_fills()
    assert len(fills) == 1
    assert fills[0].fill_price == 105.0
    assert fills[0].slippage == 5.0