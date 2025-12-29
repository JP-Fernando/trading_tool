# Trading Tool con Python y C++20 {#mainpage}


[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![C++ 20](https://img.shields.io/badge/C++-20-red.svg)](https://en.cppreference.com/w/cpp/20)

Este proyecto es un **motor de trading híbrido** diseñado para ofrecer máxima eficiencia. 
Combina la flexibilidad de **Python** para la gestión de datos y estrategias con un núcleo 
de **C++20** altamente optimizado para el cálculo de indicadores técnicos.

---

## 🚀 Características Principales

* **C++20 Core:** Cálculo de indicadores (SMA, EMA, RSI, MACD, Bollinger Bands) 
implementado con algoritmos de una sola pasada (*single-pass*) y punteros crudos para evitar latencia.
* **Ganancia Real:** Rendimiento hasta **5x superior** a las implementaciones estándar basadas puramente en Python/Pandas.
* **Interoperabilidad:** Integración fluida mediante `pybind11`, permitiendo el paso de arrays de NumPy sin copias innecesarias.
* **Estructura Profesional:** Arquitectura modular lista para escalado a trading de alta frecuencia (HFT).

---

## Estructura del Proyecto

```text
trading_tool/
├── src/
│   └── trading_core.cpp    # Implementación C++ de indicadores (SMA, EMA, RSI, etc.)
├── trading_bot/
│   ├── __init__.py
│   ├── engine.py           # Clase TradingEngine (Lógica principal)
│   └── monitor.py          # Script de monitoreo en tiempo real
├── notebooks/
│   └── benchmark.ipynb     # Comparativa de rendimiento Python vs C++
├── setup.py                # Configuración de compilación de la extensión C++
├── requirements.txt        # Dependencias de Python
└── README.md
```

---

## Instalación y Compilación

### Requisitos previos
* Compilador C++ compatible con el estándar **C++20** (GCC 10+, Clang 10+ o MSVC 2019+).
* Python 3.10 o superior.

### Pasos
1. Clonar el repositorio:
    ```bash
    git clone https://github.com/JP-Fernando/trading_tool.git
    cd trading_tool
    ```

2. Instalar y compilar:
    ```bash
    pip install -e .
    ```

---

## Uso básico

### Backtesting

Ejemplo en `research.ipynb`:

```python
from trading_bot.engine import TradingEngine

# Inicializar motor
engine = TradingEngine()

# Obtener datos de mercado
df = engine.fetch_data(symbol='BTC/USDT', timeframe='1h')

# Calcular indicadores (SMA, EMA, RSI, MACD, BB) en el Core de C++
df = engine.add_indicators(
    df, 
    sma_window=20, 
    rsi_window=14, 
    bb_window=20,
    macd_fast=12,
    macd_slow=26
)

print(df.tail())
```


---

## Roadmap
- [ ] **WebSockets Integration:** Soporte para streaming de datos en tiempo real.
- [ ] **Advanced Indicators:** Implementación de Ichimoku Cloud y ADX en C++.
- [ ] **Backtesting Engine:** Motor de ejecución de órdenes simuladas con gestión de slippage.
- [ ] **ML Integration:** Conexión con modelos de PyTorch para predicción de señales.

--- 

## Licencia

MIT License - Uso libre para fines educativos y comerciales.

---

## Descargo de responsabilidad (Disclaimer)

Este bot **no opera con dinero real**. Solo realiza backtesting y simulaciones. 
Sus fines son plenamente educativos.

**No utilices este bot con capital real** sin realizar antes pruebas
exhaustivas en entornos de simulación (`Testnet`).
Aun así, esta herramienta se ofrece **sin garantías** y el autor no 
se hace responsable de su uso por terceros.

El trading financiero conlleva un riesgo significativo de pérdida de capital.
**Invierte solo el capital que estés dispuesto a perder.**