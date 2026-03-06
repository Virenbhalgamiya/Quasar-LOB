import pandas as pd
import numpy as np

class BacktestEngine:
    def __init__(self, data_path: str, initial_capital: float = 100000.0):
        self.data_path = data_path
        self.initial_capital = initial_capital
        self.trades = []
        self.positions = []
        
    def load_data(self):
        # Expecting a CSV with columns: timestamp_ns, price, qty, side
        try:
            self.df = pd.read_csv(self.data_path)
            self.df['timestamp'] = pd.to_datetime(self.df['timestamp_ns'], unit='ns')
            self.df.set_index('timestamp', inplace=True)
            print(f"Loaded {len(self.df)} market events.")
        except Exception as e:
            print(f"Failed to load data: {e}")
            self.df = pd.DataFrame()
            
    def run_strategy(self, strategy_func):
        # Simulating event driven strategy loop on historical data
        current_pos = 0
        cash = self.initial_capital
        
        for index, row in self.df.iterrows():
            signal = strategy_func(row)
            
            # Simple execution model: execute immediately at current price
            if signal == 'BUY':
                qty_to_buy = 10  # fixed size for simple backtest
                cost = qty_to_buy * row['price']
                if cash >= cost:
                    cash -= cost
                    current_pos += qty_to_buy
                    self.trades.append({'time': index, 'side': 'BUY', 'price': row['price'], 'qty': qty_to_buy})
            
            elif signal == 'SELL' and current_pos > 0:
                qty_to_sell = 10
                revenue = qty_to_sell * row['price']
                cash += revenue
                current_pos -= qty_to_sell
                self.trades.append({'time': index, 'side': 'SELL', 'price': row['price'], 'qty': qty_to_sell})
                
            # Mark to market portfolio value
            mtm_value = cash + (current_pos * row['price'])
            self.positions.append(mtm_value)

    def calculate_metrics(self):
        if not self.positions:
            return {"PnL": 0, "Sharpe": 0, "MaxDrawdown": 0}
            
        portfolio = pd.Series(self.positions)
        
        # PnL
        final_value = portfolio.iloc[-1]
        pnl = final_value - self.initial_capital
        
        # Returns
        returns = portfolio.pct_change().dropna()
        
        # Sharpe Ratio (Assuming 0% risk free rate, daily annualized if data was daily, but here we assume tick data)
        # For tick data, standard dev is small. We will calculate a simple per-trade sharpe
        if returns.std() == 0:
            sharpe = 0.0
        else:
            sharpe = (returns.mean() / returns.std()) * np.sqrt(252 * 1000) # Pseudo-annualization factor
            
        # Maximum Drawdown
        cum_ret = (1 + returns).cumprod()
        rolling_max = cum_ret.cummax()
        drawdown = (cum_ret - rolling_max) / rolling_max
        max_drawdown = drawdown.min()
        
        return {
            "Initial Capital": self.initial_capital,
            "Final Value": final_value,
            "Total PnL": pnl,
            "Sharpe Ratio": round(sharpe, 4),
            "Max Drawdown (%)": round(max_drawdown * 100, 2),
            "Total Trades": len(self.trades)
        }

if __name__ == "__main__":
    # Dummy creation for structural completeness
    with open("dummy_data.csv", "w") as f:
        f.write("timestamp_ns,price,qty,side\n")
        f.write("1672531200000000000,100,10,BUY\n")
        f.write("1672531201000000000,105,10,SELL\n")
        f.write("1672531202000000000,102,10,BUY\n")
        f.write("1672531203000000000,108,10,SELL\n")
        
    engine = BacktestEngine("dummy_data.csv")
    engine.load_data()
    
    # Simple Momentum strategy
    last_price = 0
    def momentum_strategy(row):
        global last_price
        signal = None
        if last_price != 0:
            if row['price'] > last_price + 2:
                signal = 'BUY'
            elif row['price'] < last_price - 2:
                signal = 'SELL'
        last_price = row['price']
        return signal
        
    engine.run_strategy(momentum_strategy)
    metrics = engine.calculate_metrics()
    
    print("\n--- Backtest Results ---")
    for k, v in metrics.items():
        print(f"{k}: {v}")
