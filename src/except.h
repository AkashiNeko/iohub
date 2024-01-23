// except.h

#pragma once
#ifndef IOHUB_EXCEPT_H_
#define IOHUB_EXCEPT_H_

#include <string>
#include <exception>                                                                  \

namespace iohub {

class IOHubExcept : public std::exception {
    std::string except_msg_;
public:
    explicit IOHubExcept(const std::string& msg) : except_msg_(msg) {}
    explicit IOHubExcept(std::string&& msg) : except_msg_(std::move(msg)) {}
    virtual ~IOHubExcept() override = default;
    virtual const char* what() const noexcept override {
        return except_msg_.c_str();
    }
};

// throw exceptions
#if __cplusplus >= 201703L

template <class ExceptType = IOHubExcept, class ...Args>
inline void assert_throw(bool condition, const Args&... args) {
    if (condition) return;
    std::string s;
    ((s += args), ...);
    throw ExceptType(std::move(s));
}

#else // 201103L <= __cplusplus < 201703L

inline void append_string_(std::string&) {}

template <class T, class ...Args>
inline void append_string_(std::string& s, const T& arg, const Args&... args) {
    s += arg;
    append_string_(s, args...);
}

template <class ExceptType = IOHubExcept, class ...Args>
inline void assert_throw(bool condition, const Args&... args) {
    if (condition) return;
    std::string s;
    append_string_(s, args...);
    throw ExceptType(std::move(s));
}

#endif // __cplusplus

} // namespace iohub

#endif // IOHUB_EXCEPT_H_
