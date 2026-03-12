#include <iostream>
#include <map>
#include <deque>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <cmath>

using namespace std;

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
    string side;
    Price price;
    int quantity;
    OrderType type;

    Order() : type(LIMIT) {}
};

class OrderBook {

private:

    map<Price, deque<Order>, greater<Price> > bids;
    map<Price, deque<Order> > asks;

    unordered_map<string, pair<string, Price> > orderIndex;

    long long ordersProcessedCount = 0;

public:

    void processOrder(Order order) {

        ordersProcessedCount++;

        if (orderIndex.find(order.id) != orderIndex.end())
            return;

        if (order.type == FOK) {
            if (!canFullyFill(order)) {
                printBBO();
                return;
            }
        }

        if (order.side == "BUY")
            matchBuy(order);
        else
            matchSell(order);

        if (order.quantity > 0 && order.type == LIMIT)
            addToBook(order);

        printBBO();
    }

    void cancel(const string& id) {

        unordered_map<string, pair<string, Price> >::iterator it = orderIndex.find(id);

        if (it == orderIndex.end())
            return;

        string side = it->second.first;
        Price price = it->second.second;

        if (side == "BUY") {

            map<Price, deque<Order>, greater<Price> >::iterator mapIt = bids.find(price);

            if (mapIt != bids.end()) {

                deque<Order>& queue = mapIt->second;

                for (deque<Order>::iterator qIt = queue.begin(); qIt != queue.end(); ++qIt) {

                    if (qIt->id == id) {
                        queue.erase(qIt);
                        break;
                    }
                }

                if (queue.empty())
                    bids.erase(mapIt);
            }

        } else {

            map<Price, deque<Order> >::iterator mapIt = asks.find(price);

            if (mapIt != asks.end()) {

                deque<Order>& queue = mapIt->second;

                for (deque<Order>::iterator qIt = queue.begin(); qIt != queue.end(); ++qIt) {

                    if (qIt->id == id) {
                        queue.erase(qIt);
                        break;
                    }
                }

                if (queue.empty())
                    asks.erase(mapIt);
            }
        }

        orderIndex.erase(id);

        printBBO();
    }

    void printBook() {

        cout << "--- Book ---" << endl;

        int count = 0;

        if (asks.empty())
            cout << "ASK: (empty)" << endl;
        else {

            for (map<Price, deque<Order> >::const_iterator it = asks.begin(); it != asks.end(); ++it) {

                Price price = it->first;
                const deque<Order>& queue = it->second;

                int total = 0;

                for (deque<Order>::const_iterator q = queue.begin(); q != queue.end(); ++q)
                    total += q->quantity;

                cout << "ASK: " << priceToString(price) << " x " << total << endl;

                count++;

                if (count == 5)
                    break;
            }
        }

        count = 0;

        if (bids.empty())
            cout << "BID: (empty)" << endl;
        else {

            for (map<Price, deque<Order>, greater<Price> >::const_iterator it = bids.begin(); it != bids.end(); ++it) {

                Price price = it->first;
                const deque<Order>& queue = it->second;

                int total = 0;

                for (deque<Order>::const_iterator q = queue.begin(); q != queue.end(); ++q)
                    total += q->quantity;

                cout << "BID: " << priceToString(price) << " x " << total << endl;

                count++;

                if (count == 5)
                    break;
            }
        }
    }

    void printStats(double duration) {

        if (duration <= 0)
            return;

        long long rate = static_cast<long long>(ordersProcessedCount / duration);

        cout << "Processed "
             << ordersProcessedCount
             << " orders in "
             << fixed << setprecision(3)
             << duration
             << "s ("
             << rate
             << " orders/sec)"
             << endl;
    }

private:

    bool canFullyFill(const Order& order) {

        int remaining = order.quantity;

        if (order.side == "BUY") {

            for (map<Price, deque<Order> >::const_iterator it = asks.begin(); it != asks.end(); ++it) {

                Price price = it->first;
                const deque<Order>& queue = it->second;

                if (order.price != 0 && price > order.price)
                    break;

                for (deque<Order>::const_iterator q = queue.begin(); q != queue.end(); ++q) {

                    remaining -= q->quantity;

                    if (remaining <= 0)
                        return true;
                }
            }

        } else {

            for (map<Price, deque<Order>, greater<Price> >::const_iterator it = bids.begin(); it != bids.end(); ++it) {

                Price price = it->first;
                const deque<Order>& queue = it->second;

                if (order.price != 0 && price < order.price)
                    break;

                for (deque<Order>::const_iterator q = queue.begin(); q != queue.end(); ++q) {

                    remaining -= q->quantity;

                    if (remaining <= 0)
                        return true;
                }
            }
        }

        return false;
    }

    void matchBuy(Order& buy) {

        while (buy.quantity > 0 && !asks.empty()) {

            map<Price, deque<Order> >::iterator bestAsk = asks.begin();

            if (buy.price != 0 && bestAsk->first > buy.price)
                break;

            deque<Order>& queue = bestAsk->second;

            Order& sell = queue.front();

            int tradeQty = min(buy.quantity, sell.quantity);

            cout << "TRADE "
                 << buy.id << " "
                 << sell.id << " "
                 << priceToString(bestAsk->first)
                 << " "
                 << tradeQty << endl;

            buy.quantity -= tradeQty;
            sell.quantity -= tradeQty;

            if (sell.quantity == 0) {
                orderIndex.erase(sell.id);
                queue.pop_front();
            }

            if (queue.empty())
                asks.erase(bestAsk);
        }
    }

    void matchSell(Order& sell) {

        while (sell.quantity > 0 && !bids.empty()) {

            map<Price, deque<Order>, greater<Price> >::iterator bestBid = bids.begin();

            if (sell.price != 0 && bestBid->first < sell.price)
                break;

            deque<Order>& queue = bestBid->second;

            Order& buy = queue.front();

            int tradeQty = min(sell.quantity, buy.quantity);

            cout << "TRADE "
                 << buy.id << " "
                 << sell.id << " "
                 << priceToString(bestBid->first)
                 << " "
                 << tradeQty << endl;

            sell.quantity -= tradeQty;
            buy.quantity -= tradeQty;

            if (buy.quantity == 0) {
                orderIndex.erase(buy.id);
                queue.pop_front();
            }

            if (queue.empty())
                bids.erase(bestBid);
        }
    }

    void addToBook(const Order& order) {

        if (order.side == "BUY")
            bids[order.price].push_back(order);
        else
            asks[order.price].push_back(order);

        orderIndex[order.id] = make_pair(order.side, order.price);
    }

    void printBBO() {

        if (bids.empty() && asks.empty())
            return;

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

        if (line.empty())
            continue;

        stringstream ss(line);

        string first;

        if (!(ss >> first))
            continue;

        if (first == "CANCEL") {

            string id;

            if (ss >> id)
                book.cancel(id);

        } else {

            Order order;

            order.id = first;

            string side;

            if (!(ss >> side))
                continue;

            transform(side.begin(), side.end(), side.begin(), ::toupper);

            order.side = side;

            string priceStr;

            if (!(ss >> priceStr))
                continue;

            order.price = stringToPrice(priceStr);

            if (!(ss >> order.quantity))
                continue;

            string extra;

            if (ss >> extra) {

                transform(extra.begin(), extra.end(), extra.begin(), ::toupper);

                if (extra == "IOC")
                    order.type = IOC;
                else if (extra == "FOK")
                    order.type = FOK;

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