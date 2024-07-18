//
// Created by zoravur on 6/21/24.
//

#ifndef STORE_HPP
#define STORE_HPP

#include "Subject.hpp"

template <typename T>
class Store : public Subject<T, Store<T>> {
    T val;
public:
    Store() = default;

    explicit Store(const T& val): val(val) {}

    void set(T new_val) {
        if (new_val != val) {
            val = new_val;
            this->notify(new_val, *this);
        }
    }

    T get() const {
        return val;
    }

};



#endif //STORE_HPP
