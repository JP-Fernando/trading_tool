from trading_bot.trading_tool import backtest as bt
from trading_bot.trading_tool import events as ev


def run_simulation_scenario():
    """Helper function to run an isolated simulation scenario."""
    queue = bt.EventQueue()
    model = lambda info: info.mid_price * 1.001

    execution = bt.ExecutionEngine(queue, model)
    engine = bt.BacktestEngine(queue, execution)

    for i in range(10):
        timestamp = ev.make_timestamp(i * 1000)
        tick = ev.TickEvent(
            timestamp,
            "AAPL",
            150.0 + i,
            151.0 + i,
            10.0,
            10.0,
            150.5 + i,
            1.0,
        )
        engine.push_event(tick)

        order = ev.OrderEvent(
            i,
            ev.make_timestamp(i * 1000 + 1),
            "AAPL",
            ev.Side.BUY,
            1.0,
            0.0,
            ev.OrderStatus.SUBMITTED,
            "deterministic_strat",
        )
        engine.push_event(order)

    engine.run()
    return execution.get_fills()


def test_simulation_determinism():
    """
    Assert that two identical simulation runs produce
    exactly the same results (Bit-perfect reproducibility).
    """
    results_run_1 = run_simulation_scenario()
    results_run_2 = run_simulation_scenario()

    assert len(results_run_1) == 10
    assert len(results_run_2) == 10

    for fill_1, fill_2 in zip(results_run_1, results_run_2):
        assert fill_1.order_id == fill_2.order_id
        assert fill_1.fill_price == fill_2.fill_price
        assert fill_1.timestamp == fill_2.timestamp
        assert fill_1.commission == fill_2.commission