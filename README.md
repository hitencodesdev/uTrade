# uTrade - In-Memory Limit Order Book

A high-performance C++ implementation of a limit order book that supports LIMIT, MARKET, Immediate-or-Cancel (IOC), and Fill-or-Kill (FOK) orders with price-time priority matching.

## Code Explanation & Line-by-Line Significance

### Header Includes
- **Lines 1-10**: Basic libraries for input/output (`iostream`, `iomanip`), data structures (`map`, `deque`, `unordered_map`), and utilities (`string`, `sstream`, `algorithm`, `chrono`, `cmath`).

### Type Definitions & Utilities
- **Line 13**: `typedef long long Price;` - Prices are stored as `long long` integers to avoid floating-point precision issues (fixed-point arithmetic).
- **Lines 15-23**: `stringToPrice` - Safely converts string input (e.g., "10.50") into a scaled integer (1050).
- **Lines 25-29**: `priceToString` - Formats the scaled integer back into a standard two-decimal string.

### Core Data Structures
- **Line 31**: `enum OrderType` - Defines the behavior of orders: `LIMIT`, `MARKET`, `IOC`, `FOK`.
- **Lines 33-41**: `struct Order` - Represents an order entity with ID, side, price, quantity, and type. Line 40 is the constructor defaulting to `LIMIT`.

### OrderBook Class
- **Line 47**: `map<Price, deque<Order>, greater<Price> > bids;` - Buy orders sorted highest price first for price priority.
- **Line 48**: `map<Price, deque<Order> > asks;` - Sell orders sorted lowest price first.
- **Line 50**: `unordered_map<string, pair<string, Price> > orderIndex;` - Maps order IDs to their details for $O(1)$ lookup during cancellation.

### Processing Logic
- **Lines 56-79**: `processOrder` - The workflow for a new order (Matching -> Filling -> Resting).
- **Lines 81-135**: `cancel` - Robust cancellation using iterators to cleanly remove orders from the maps and the index.
- **Lines 137-190**: `printBook` - Displays top 5 asks and top 5 bids using `const_iterator` for C++11 compatibility.
- **Lines 192-208**: `printStats` - Calculates and prints the performance throughput (orders per second).

### Matching Engine
- **Lines 212-256**: `canFullyFill` - Essential for `FOK` (Fill-or-Kill) orders; checks if enough liquidity exists before executing.
- **Lines 258-291**: `matchBuy` - Matches a buy order against the resting asks. Trades happen at the maker's price.
- **Lines 293-326**: `matchSell` - Matches a sell order against resting bids.
- **Lines 328-336**: `addToBook` - Adds unexecuted limit order quantity to the book.
- **Lines 338-347**: `printBBO` - Prints the real-time "Best Bid / Best Offer".

### Main Function
- **Lines 350-434**: `main` - The driver program.
    - **Lines 352-353**: Optimizes performance by disabling I/O synchronization.
    - **Lines 361-423**: The main loop: reads inputs, parses commands (CANCEL or ORDER), and handles order types (MARKET, IOC, FOK).
    - **Line 425**: Final book state output.
    - **Lines 431**: Output processing speed measurement.

## How to Run
1. Compile: `g++ -O3 utrade.cpp -o utrade.exe`
2. Run with sample input: `Get-Content sample_input.txt | ./utrade.exe`

## Performance
The engine uses efficient STL containers to achieve high throughput, measured in orders per second at the end of execution.
