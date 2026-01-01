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
  implementado con algoritmos de una sola pasada (*single-pass*).
* **Interoperabilidad:** Integración mediante `pybind11` para pasar arrays de NumPy
  sin copias innecesarias.
* **Backtesting orientado a eventos:** `EventQueue`, `BacktestEngine` y
  `ExecutionEngine` con modelo de slippage configurable.
* **Multithreaded MarketManager:** Procesamiento asíncrono de múltiples activos
  con un `ThreadPool` nativo en C++.
* **Thread-Safe Architecture:** Uso de `std::shared_mutex` y un `Logger` centralizado
  para integridad entre hilos.


---

## Estructura del Proyecto

```text
trading_tool/
├── include/
│   ├── backtest/           # EventQueue, BacktestEngine, ExecutionEngine
│   ├── core/               # MarketManager, Indicators, ThreadPool, Events
│   └── utils/              # Logger
├── src/
│   ├── backtest/           # Implementación del motor de backtest en C++
│   ├── core/               # Implementación C++ de la lógica de negocio
│   ├── utils/              # Implementación de utilidades
│   └── bindings.cpp        # Definición de módulos Pybind11
├── trading_bot/
│   ├── engine.py           # Lógica de alto nivel y backtest en Python
│   ├── monitor.py          # Monitor de WebSockets en tiempo real (asyncio)
│   └── trading_tool.py     # Atajos para eventos/backtest desde Python
├── tests/                  # Suite de tests (Pytest)
├── build.sh                # Script de compilación C++ + pybind11
├── setup.py                # Configuración de instalación
```

---

## Instalación y Compilación

### Requisitos previos
* Compilador C++ compatible con el estándar **C++20** (GCC 10+, Clang 10+ o MSVC 2019+).
* Python 3.10 o superior y `pip`.

### Pasos
1. Clonar el repositorio:
    ```bash
    git clone https://github.com/JP-Fernando/trading_tool.git
    cd trading_tool
    ```

2. Instalar dependencias:
    ```bash
    pip install -r requirements.txt
    ```

3. Compilar la extensión C++:
    ```bash
    chmod +x build.sh
    ./build.sh Release
    ```

4. Generar documentación (opcional):
    ```bash
    doxygen Doxyfile
    ```

---

## Uso básico

### Cálculo de indicadores en Python

```python
import numpy as np
from trading_bot import trading_core

prices = np.array([100, 101, 102, 99, 98, 105], dtype=np.float64)
sma = trading_core.calculate_sma(prices, 3)
ema = trading_core.calculate_ema(prices, 3)
```

### Calidad y validación

El proyecto cuenta con una suite de pruebas dividida en dos niveles para garantizar la estabilidad del sistema:

1. Pruebas de Integración (Python)

    Validan la comunicación entre Python y C++, indicadores, backtesting y concurrencia:

    ```bash
    # Instalar dependencias de test
    pip install -e ".[test]"

    # Ejecutar tests
    pytest 
    ```

2. Pruebas Unitarias (C++ Core)

    Si has compilado en modo Debug y tienes GTest instalado, puedes ejecutar los tests nativos:

    ```bash
    cd build
    ctest --output-on-failure
    ```


---

### Uso del Monitor en Tiempo Real

El script `monitor.py` utiliza `ccxt.pro` para conectar con
WebSockets de exchanges y delegar el análisis al núcleo de C++:

```python
import asyncio
from trading_bot import trading_core

async def main():
    manager = trading_core.MarketManager(num_threads=4)
    # ... configuración de websockets ...
    # El motor procesará los ticks en background y emitirá señales vía Logger
```

---

## Roadmap

- [  ] **Persistent Storage:** Base de datos de alta velocidad para ticks (TimeScaleDB/InfluxDB).
- [  ] **Advanced Indicators:** Implementación de Ichimoku Cloud y ADX en C++.
- [  ] **Execution Module:** Integración con APIs de trading para ejecución de órdenes.
- [  ] **ML Integration:** Conexión con modelos de PyTorch para predicción de señales.

--- 

## Licencia

MIT License - Uso libre para fines educativos y comerciales.

---

## Descargo de responsabilidad (Disclaimer)

Este bot **no opera con dinero real**. 
Sus fines son plenamente educativos.

**No utilices este bot con capital real** sin realizar antes pruebas
exhaustivas en entornos de simulación (`Testnet`).
Aun así, esta herramienta se ofrece **sin garantías** y el autor no 
se hace responsable de su uso por terceros.

El trading financiero conlleva un riesgo significativo de pérdida de capital.
**Invierte solo el capital que estés dispuesto a perder.**