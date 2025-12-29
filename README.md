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
* **Interoperabilidad:** Integración fluida mediante `pybind11`, 
permitiendo el paso de arrays de NumPy sin copias innecesarias.
* **Zero-Copy Integration:** Paso de arrays de NumPy a C++ optimizado para minimizar 
la latencia en el cálculo de indicadores (RSI, MACD, Bollinger Bands, etc.).
* **Multithreaded MarketManager:** Procesamiento asíncrono de múltiples 
activos simultáneamente utilizando un `ThreadPool` nativo en C++.
* **Thread-Safe Architecture:** Implementación de bloqueos de lectura/escritura 
(`std::shared_mutex`) para garantizar la integridad de los datos entre hilos.
* **Ganancia Real:** Rendimiento hasta **5x superior** a las implementaciones estándar basadas puramente en Python/Pandas.

---

## Estructura del Proyecto

```text
trading_tool/
├── include/
│   ├── core/      # Cabeceras del motor (MarketManager, Indicators, ThreadPool)
│   └── utils/             # Utilidades transversales (Logger)
├── src/
│   ├── core/      # Implementación C++ de la lógica de negocio
│   ├── utils/             # Implementación de utilidades
│   └── bindings.cpp       # Definición de módulos Pybind11
├── trading_bot/
│   ├── engine.py          # Lógica de alto nivel
│   └── monitor.py         # Monitor de WebSockets en tiempo real (Asyncio)
├── tests/                 # Suite de tests unitarios e integración (Pytest)
├── docs/                  # Documentación generada (Doxygen)
├── setup.py               # Compilación de la extensión C++
└── README.md
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

2. Instalar y compilar:
    ```bash
    pip install -e .
    ```

3. Generar documentación (opcional):
    ```bash
    doxygen Doxyfile
    ```

---

## Uso básico

### Calidad y validación

El proyecto utiliza `pytest` para asegurar la estabilidad de la integración C++/Python y la concurrencia:

```bash
pytest tests/
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

- [  ] **Backtesting Engine:** Motor de ejecución de órdenes simuladas con gestión de slippage.
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