//
// Created by Meow on 26.01.2019.
//

#ifndef UNTITLED2_FUNCTION_H
#define UNTITLED2_FUNCTION_H

#include <bits/unique_ptr.h>

#include <memory>
#include <utility>
#include <iostream>
#include <cstring>

const size_t SMALL_OBJECT = 64;

template <class T>
class function;

template <class R, class... Args>
class function<R(Args...)> {
public:
    function() noexcept : func(nullptr), small(false) {}

    explicit function(std::nullptr_t) noexcept : func(nullptr), small(false) {}

    function(const function &other) : func(nullptr), small(other.small) {
        if (small) {
            ((base*)other.buff)->copy_small((char*)buff);
        } else {
            func = std::move(other.func->copy());
        }
    }

    function(function &&other) noexcept : func(nullptr), small(false) {
        std::swap(buff, other.buff);
        std::swap(small, other.small);
        other.small = false;
        //other.func.reset(nullptr);
    }

    template<class F>
    function(F f) {
        if (sizeof(holder<F>(std::move(f))) <= SMALL_OBJECT) {
            small = true;
            new(buff) holder<F>(std::move(f));
        } else {
            small = false;
            new(buff) std::unique_ptr<holder<F>>(new holder<F>(std::move(f)));
        }
    }

    ~function() {
        if (small) {
            ((base*)buff)->~base();
        } else {
            func.reset(nullptr);
        }
    }

    function& operator=(const function &other) {
        function tmp(other);
        swap(tmp);
        return *this;
    }

    function& operator=(function &&other) noexcept {
        auto tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    void swap(function &other) noexcept {
        std::swap(buff, other.buff);
        std::swap(small,other.small);
    }

    explicit operator bool() const noexcept {
        return (small || (func.get() != 0));
    }

    R operator()(Args &&... args) {
        if (small) {
            return ((base*)buff)->invoke(std::forward<Args>(args)...);
        }
        return func->invoke(std::forward<Args>(args)...);
    }

private:
    class base {
    public:
        base() = default;

        virtual ~base() = default;

        virtual R invoke(Args...) = 0;

        virtual std::unique_ptr<base> copy() = 0;

        virtual void copy_small(void*) = 0;
    };

    template <typename F>
    class holder : public base {
    public:
        explicit holder(F func) : f(func) {}

        ~holder() override = default;

        R invoke(Args... args) {
            return f(std::forward<Args>(args)...);
        }

        std::unique_ptr<base> copy() override {
            return std::make_unique<holder<F>>(f);
        }

        void copy_small(void *pt) {
            new(pt) holder<F>(f);
        }

    private:
        F f;
    };

    union {
        std::unique_ptr<base> func;
        char buff[SMALL_OBJECT];
    };

    bool small;
};

#endif //UNTITLED2_FUNCTION_H
