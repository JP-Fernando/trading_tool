from trading_bot.trading_tool import backtest as bt
from trading_bot.trading_tool import events as ev


def test_event_queue_chronological_order():
    """
    Checks order of priority queue by ascending timestamp
    (i.e. oldest first)
    """
    queue = bt.EventQueue()

    t_late = ev.make_timestamp(300)
    t_early = ev.make_timestamp(100)
    t_mid = ev.make_timestamp(200)

    event_1 = ev.TickEvent(t_late, "BTC", 100.0, 101.0, 1.0, 1.0, 100.5, 1.0)
    event_2 = ev.TickEvent(t_early, "BTC", 90.0, 91.0, 1.0, 1.0, 90.5, 1.0)
    event_3 = ev.TickEvent(t_mid, "BTC", 95.0, 96.0, 1.0, 1.0, 95.5, 1.0)

    queue.push(event_1)
    queue.push(event_2)
    queue.push(event_3)

    assert queue.size() == 3

    popped_1 = queue.pop()
    popped_2 = queue.pop()
    popped_3 = queue.pop()

    assert popped_1.timestamp < popped_2.timestamp < popped_3.timestamp
    assert queue.empty()
