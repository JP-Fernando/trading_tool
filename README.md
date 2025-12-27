# Trading Tool con Python

Este proyecto es un **bot de trading en Python** que permite:
- Descargar datos históricos de criptomonedas usando **ccxt**.
- Calcular indicadores técnicos (SMA) y generar señales de compra/venta.
- Realizar **backtesting** de estrategias simples.
- Visualizar resultados y optimizar parámetros de la estrategia.

---


## Estructura del Proyecto

```text
trading_tool/
├── engine.py          # Motor de trading (fetch de datos, indicadores, señales, backtest)
├── monitor.py         # Monitor para escuchar datos y generar señales en tiempo real
├── research.ipynb     # Notebook para experimentación y optimización de estrategias
├── requirements.txt   # Dependencias del proyecto
└── README.md          # Documentación
```

---

## Instalación

1. Clonar el repositorio:
    ```bash
    git clone https://github.com/tuusuario/trading_tool.git
    cd trading_tool
    ```

2. Crear entorno con Anaconda (opcional pero recomendado):
    ```bash
    conda create -n trading_tool python=3.11
    conda activate trading_tool
    ```

3. Instalar dependencias:
    ```bash
    pip install -r requirements.txt
    ```

---

## Uso básico

### Backtesting

Ejemplo en `research.ipynb`:

```python
from engine import TradingEngine

bot = TradingEngine()
df = bot.fetch_data(timeframe='1h', limit=500)
df = bot.add_indicators(df)
df = bot.get_signals(df)

results = bot.run_backtest(df, initial_balance=1000)
print(f"Total Return: {results['total_return_pct']}%")
```

### Monitor de señales

```bash
python monitor.py
```

Esto imprimirá en consola las últimas señales generadas para el símbolo configurado.

### Optimización de parámetros

Puedes probar distintas ventanas de SMA para encontrar la que maximice tu retorno:

```python
ventanas = [5, 10, 20, 50, 100, 200]
# Ejecutar optimización en research.ipynb
```

## Advertencia
Este bot **no opera con dinero real**. Solo realiza backtesting y simulaciones. 
Para trading real, usar Binance Testnet y probar cuidadosamente.