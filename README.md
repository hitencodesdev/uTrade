# uTrade - In-Memory Limit Order Book

uTrade is a fast C++ limit order book that supports:

- LIMIT orders
- MARKET orders
- IOC (Immediate-or-Cancel)
- FOK (Fill-or-Kill)

It matches trades using price-time priority and keeps the order book in memory for quick execution.

## Overview

This project simulates a trading engine that accepts incoming orders, matches them against existing liquidity, and either fills, cancels, or rests the remaining quantity.

It is designed to be simple to follow while still using efficient C++ containers for performance.

## Key Components

### 1. Price handling  
- Prices are stored as `long long` integers to avoid floating-point issues.
- Values like `10.50` are converted to integer ticks such as `1050`.
- The engine converts them back to readable strings when printing output.

### 2. Order types
The project defines several order behaviors:

- `LIMIT`: waits for a matching price if needed
- `MARKET`: executes immediately against the best available liquidity
- `IOC`: executes immediately and cancels any unmatched portion
- `FOK`: executes only if the full quantity can be filled immediately; otherwise it cancels

### 3. Core book structure
The order book keeps buy and sell orders in separate sorted containers:

- Bids are arranged from highest price to lowest price.
- Asks are arranged from lowest price to highest price.
- An index maps order IDs to their details so cancellation can happen quickly.

### 4. Matching logic
The engine follows this basic process for each incoming order:

1. Check if the order can match with the opposite side.
2. Fill as much as possible against available resting orders.
3. If any quantity remains, either rest it or cancel it based on order type.

## Code Structure

### Header includes
The file includes standard libraries for:

- input and output
- data containers
- strings and formatting
- time and math utilities

### Order model
The `Order` structure stores:

- order ID
- side (buy or sell)
- price
- quantity
- order type

### `OrderBook` class
This class handles:

- order processing
- cancellation
- book display
- trade matching
- best bid / best offer reporting
- throughput statistics

### Main program flow
The `main` function:

- disables slow I/O synchronization for speed
- reads incoming commands
- parses order and cancel requests
- executes matching logic
- prints the final book state
- reports orders processed per second

## How to Run

1. Compile the program:
   `g++ -O3 utrade.cpp -o utrade.exe`

2. Run it with the sample input:
   `Get-Content sample_input.txt | ./utrade.exe`

## Performance

The engine uses efficient STL containers and optimized matching logic to process a large number of orders quickly. At the end of execution, it reports throughput in orders per second.

## Notes

This project is a good example of a lightweight in-memory matching engine. It is meant to be readable and performant, with each section focused on a specific part of the order book workflow.
