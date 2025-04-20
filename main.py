import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# pandas used for data analysis
# mathplot and seaborn are used for visuals like graphs
sns.set(style='whitegrid')

# opening our file which is linked with atm
df = pd.read_csv(r"C:\Users\lenovo\Desktop\ali-hussain\atm_transactions.csv")
df['Timestamp'] = pd.to_datetime(df['Timestamp'])

print("Previewing the data:\n", df.head())

print("\nDescriptive statistics:\n", df.describe())
# Plot 1: Distribution of Transaction Amounts

plt.figure(figsize=(8, 5))
sns.histplot(df['Amount'], bins=10, kde=True, color='skyblue')
plt.title('How Are Transaction Amounts Distributed?')
plt.xlabel('Transaction Amount')
plt.ylabel('Frequency')
plt.grid(True)
plt.show()

# Group by date and sum the amounts
df_daily = df.groupby(df['Timestamp'].dt.date)['Amount'].sum().reset_index()
df_daily.columns = ['Date', 'Total_Amount']

plt.figure(figsize=(10, 5))
sns.lineplot(x='Date', y='Total_Amount', data=df_daily, marker='o')
plt.title('Total Withdrawals per Day')
plt.xlabel('Date')
plt.ylabel('Total Withdrawn Amount')
# clean plot
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()

# Transaction Amounts for Each Account

plt.figure(figsize=(8, 5))
sns.boxplot(x='AccNum', y='Amount', data=df)
plt.title('Transaction Amounts Grouped by Account')
plt.xlabel('Account Number')
plt.ylabel('Transaction Amount')
plt.xticks(rotation=45)
plt.grid(True)
plt.show()
