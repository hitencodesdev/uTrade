#include <iostream>
#include <map>
#include <deque>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <bits/stdc++.h>

using namespace std;

// Price is handled as fixed-point (long long) scaled by 100
typedef long long Price;

Price stringToPrice(const string& s) {
    if (s == "0") return 0;
    try {
        double d = stod(s);
        return static_cast<Price>(round(d * 100));
    } catch (...) {
        return 0;
    }
}

string priceToString(Price p) {
    stringstream ss;
    ss << fixed << setprecision(2) << static_cast<double>(p) / 100.0;
    return ss.str();
}

enum OrderType { LIMIT, MARKET, IOC, FOK };

struct Order {
    string id;
    string side; // BUY or SELL
    Price price;
    int quantity;
    OrderType type = LIMIT;
};

class OrderBook {
private:
    // BUY side (highest price first)
    map<Price, deque<Order>, greater<Price>> bids;

    // SELL side (lowest price first)
    map<Price, deque<Order>> asks;

    // order_id -> (side, price)
    unordered_map<string, pair<string, Price>> orderIndex;

    long long ordersProcessedCount = 0;

public:
    void processOrder(Order order) {
        ordersProcessedCount++;

        // Self-trade / Duplicate ID prevention
        if (orderIndex.find(order.id) != orderIndex.end()) {
            return;
        }

        if (order.type == FOK) {
            if (!canFullyFill(order)) {
                printBBO();
                return;
            }
        }

        if (order.side == "BUY") {
            matchBuy(order);
        } else if (order.side == "SELL") {
            matchSell(order);
        }

        // If remaining quantity and it's a resting-type order, add to book
        if (order.quantity > 0 && order.type == LIMIT) {
            addToBook(order);
        }
        
        printBBO();
    }

    void cancel(const string& id) {
        if (orderIndex.find(id) == orderIndex.end()) return;

        auto info = orderIndex[id];
        string side = info.first;
        Price price = info.second;

        if (side == "BUY") {
            auto& queue = bids[price];
            for (auto it = queue.begin(); it != queue.end(); ++it) {
                if (it->id == id) {
                    queue.erase(it);
                    break;
                }
            }
            if (queue.empty()) bids.erase(price);
        } else if (side == "SELL") {
            auto& queue = asks[price];
            for (auto it = queue.begin(); it != queue.end(); ++it) {
                if (it->id == id) {
                    queue.erase(it);
                    break;
                }
            }
            if (queue.empty()) asks.erase(price);
        }
        orderIndex.erase(id);
        printBBO();
    }

    void printBook() {
        cout << "--- Book ---" << endl;
        
        // Asks (Sells) - show top 5 levels
        int count = 0;
        if (asks.empty()) {
            cout << "ASK: (empty)" << endl;
        } else {
            for (auto const& p : asks) {
                Price price = p.first;
                auto const& queue = p.second;
                int total = 0;
                for (auto const& o : queue) total += o.quantity;
                cout << "ASK: " << priceToString(price) << " x " << total << endl;
                if (++count == 5) break;
            }
        }

        count = 0;
        // Bids (Buys) - show top 5 levels
        if (bids.empty()) {
            cout << "BID: (empty)" << endl;
        } else {
            for (auto const& p : bids) {
                Price price = p.first;
                auto const& queue = p.second;
                int total = 0;
                for (auto const& o : queue) total += o.quantity;
                cout << "BID: " << priceToString(price) << " x " << total << endl;
                if (++count == 5) break;
            }
        }
    }

    void printStats(double duration) {
        if (duration > 0) {
            cout << "Processed " << ordersProcessedCount << " orders in " << fixed << setprecision(3) << duration << "s (" 
                 << (ordersProcessedCount > 0 ? static_cast<long long>(ordersProcessedCount / duration) : 0) << " orders/sec)" << endl;
        }
    }

private:
    bool canFullyFill(const Order& order) {
        int remaining = order.quantity;
        if (order.side == "BUY") {
            for (auto const& p : asks) {
                Price price = p.first;
                auto const& queue = p.second;
                if (order.price != 0 && price > order.price) break;
                for (auto const& o : queue) {
                    remaining -= o.quantity;
                    if (remaining <= 0) return true;
                }
            }
        } else {
            for (auto const& p : bids) {
                Price price = p.first;
                auto const& queue = p.second;
                if (order.price != 0 && price < order.price) break;
                for (auto const& o : queue) {
                    remaining -= o.quantity;
                    if (remaining <= 0) return true;
                }
            }
        }
        return false;
    }

    void matchBuy(Order& buy) {
        while (buy.quantity > 0 && !asks.empty()) {
            auto bestAsk = asks.begin();
            if (buy.price != 0 && bestAsk->first > buy.price) break;

            auto& queue = bestAsk->second;
            Order& sell = queue.front();

            int tradeQty = min(buy.quantity, sell.quantity);
            // Trade price is maker price
            cout << "TRADE " << buy.id << " " << sell.id << " " << priceToString(bestAsk->first) << " " << tradeQty << endl;

            buy.quantity -= tradeQty;
            sell.quantity -= tradeQty;

            if (sell.quantity == 0) {
                orderIndex.erase(sell.id);
                queue.pop_front();
            }
            if (queue.empty()) asks.erase(bestAsk);
        }
    }

    void matchSell(Order& sell) {
        while (sell.quantity > 0 && !bids.empty()) {
            auto bestBid = bids.begin();
            if (sell.price != 0 && bestBid->first < sell.price) break;

            auto& queue = bestBid->second;
            Order& buy = queue.front();

            int tradeQty = min(sell.quantity, buy.quantity);
            // Trade price is maker price
            cout << "TRADE " << buy.id << " " << sell.id << " " << priceToString(bestBid->first) << " " << tradeQty << endl;

            sell.quantity -= tradeQty;
            buy.quantity -= tradeQty;

            if (buy.quantity == 0) {
                orderIndex.erase(buy.id);
                queue.pop_front();
            }
            if (queue.empty()) bids.erase(bestBid);
        }
    }

    void addToBook(Order order) {
        if (order.side == "BUY") {
            bids[order.price].push_back(order);
        } else {
            asks[order.price].push_back(order);
        }
        orderIndex[order.id] = {order.side, order.price};
    }

    void printBBO() {
        if (bids.empty() && asks.empty()) return;
        string bidStr = bids.empty() ? "(empty)" : priceToString(bids.begin()->first);
        string askStr = asks.empty() ? "(empty)" : priceToString(asks.begin()->first);
        cout << "BBO: " << bidStr << " / " << askStr << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    OrderBook book;
    string line;
    auto start = chrono::high_resolution_clock::now();

    while (getline(cin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string first;
        if (!(ss >> first)) continue;

        if (first == "CANCEL") {
            string id;
            if (ss >> id) book.cancel(id);
        } else {
            Order order;
            order.id = first;
            string side;
            if (!(ss >> side)) continue;
            transform(side.begin(), side.end(), side.begin(), ::toupper);
            order.side = side;

            string priceStr;
            if (!(ss >> priceStr)) continue;
            order.price = stringToPrice(priceStr);

            if (!(ss >> order.quantity)) continue;

            string extra;
            if (ss >> extra) {
                transform(extra.begin(), extra.end(), extra.begin(), ::toupper);
                if (extra == "IOC") order.type = IOC;
                else if (extra == "FOK") order.type = FOK;
            } else if (order.price == 0) {
                order.type = MARKET;
            }

            book.processOrder(order);
        }
    }

    book.printBook();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    book.printStats(diff.count());

    return 0;
}
