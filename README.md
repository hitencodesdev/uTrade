# uTrade - In-Memory Limit Order Book

A high-performance C++ implementation of a limit order book that supports LIMIT, MARKET, Immediate-or-Cancel (IOC), and Fill-or-Kill (FOK) orders with price-time priority matching.

## Code Explanation & Line-by-Line Significance

### Header Includes
- **Lines 1-10**: Includes necessary standard libraries for I/O (`iostream`, `iomanip`), data structures (`map`, `deque`, `unordered_map`), string manipulation (`sstream`, `algorithm`), and performance measurement (`chrono`, `cmath`).

### Type Definitions & Utilities
- **Line 14**: `typedef long long Price;` - Prices are handled as `long long` to implement **fixed-point arithmetic**.
- **Lines 16-24**: `stringToPrice` - Converts a string price (e.g., "100.50") to a scaled integer (10050) to avoid floating-point precision errors.
- **Lines 26-30**: `priceToString` - Formats the scaled integer back into a decimal string for output.

### Core Data Structures
- **Line 32**: `enum OrderType` - Defines supported order behaviors: `LIMIT`, `MARKET`, `IOC`, `FOK`.
- **Lines 34-40**: `struct Order` - Represents a single order with metadata (ID, Side, Price, Quantity, Type).

### OrderBook Class
- **Line 44**: `map<Price, deque<Order>, greater<Price>> bids;` - Store buy orders. Map keys (prices) are sorted descending so the best bid is always at the top (`bids.begin()`).
- **Line 47**: `map<Price, deque<Order>> asks;` - Store sell orders. Map keys are sorted ascending so the best ask is always at the top (`asks.begin()`).
- **Line 50**: `unordered_map<string, pair<string, Price>> orderIndex;` - Provides $O(1)$ lookup for order cancellation and duplicate ID check.

### Order Processing
- **Lines 55-83**: `processOrder` - The main entry point for new orders.
    - **Line 60**: Prevents duplicate Order IDs.
    - **Line 65**: Handles `FOK` logic—if the order cannot be fully filled immediately, it is discarded.
    - **Line 72-76**: Routes to the appropriate matching engine based on side.
    - **Line 79**: If it's a `LIMIT` order with remaining quantity, it's added to the book.
    - **Line 83**: Updates the Best Bid/Offer (BBO) output.

### Matching Engine
- **Lines 151-177**: `matchBuy` - Iterates through available asks.
    - **Line 153**: Matching stops if the buy price is lower than the best ask (for limit orders).
    - **Line 161**: **Trade Price Discovery** - The trade always occurs at the price of the resting order (Maker Price).
- **Lines 179-205**: `matchSell` - Symmetric logic for sell orders matching against bids.

### Support Functions
- **Lines 86-111**: `cancel` - Removes an order from the book and the index in $O(1)$ or $O(N)$ for the price level.
- **Lines 113-138**: `printBook` - Displays the top 5 bid and ask levels with aggregated quantities.
- **Lines 217-223**: `printBBO` - Outputs the real-time best bid and offer.

### Main Entry Point
- **Lines 226-270**: `main` - Optimizes I/O (`sync_with_stdio(false)`), parses stdin line-by-line, and handles command routing.
- **Line 268**: Prints final performance statistics (throughput).

## How to Run
1. Compile: `g++ -O3 utrade.cpp -o utrade.exe`
2. Run with sample input: `Get-Content sample_input.txt | ./utrade.exe`

## Performance
The engine uses efficient STL containers to achieve high throughput, measured in orders per second at the end of execution.
