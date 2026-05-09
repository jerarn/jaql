import math

import jaql


def test_python_bindings_mvp_surface():
    start = jaql.Date(2024, 1, 1)
    end = jaql.Date(2025, 1, 1)
    yf = jaql.year_fraction(start, end, jaql.DayCountConvention.Actual365Fixed)
    assert math.isclose(yf, 366.0 / 365.0, rel_tol=1e-12)

    price = jaql.black_scholes_price(jaql.OptionType.Call, 100.0, 100.0, 0.05, 0.2, 1.0)
    assert math.isclose(price, 10.4505835722, rel_tol=1e-9)

    curve = jaql.FlatYieldCurve(0.03)
    assert math.isclose(curve.discount(2.0), math.exp(-0.06), rel_tol=1e-12)
