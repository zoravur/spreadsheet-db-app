//
// Created by zoravur on 6/21/24.
//

#ifndef SUBJECT_HPP
#define SUBJECT_HPP

#include <functional>
#include <vector>

template <class... Args>
class Subject {
public:
    void notify(Args... args) {
        for (const auto& update : subscribers) {
            update(args...);
        }
    }

    size_t subscribe(const std::function<void(Args...)>& update) {
        subscribers.push_back(update);
        return subscribers.size()-1;
    }

    void unsubscribe(size_t index) {
        if (index < subscribers.size()) {
            subscribers.erase(subscribers.begin() + index);
        }
    }

private:
    std::vector<std::function<void(Args...)>> subscribers;
};



#endif //SUBJECT_HPP
