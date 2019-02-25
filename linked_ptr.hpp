//
// Created by Meow on 20.02.2019.
//

#ifndef LINKED_PTR_H
#define LINKED_PTR_H

namespace smart_ptr
{
    namespace details
    {
        class Node
        {
        public:
            Node *l = nullptr;
            Node *r = nullptr;

            Node(Node *l, Node *r) : l(l), r(r) {}
            Node() : Node(nullptr, nullptr) {}

        public:
            void attach(Node* copy)
            {
                copy->l = this;
                copy->r = r;
                if (r)
                    r->l = copy;
                r = copy;
            }

            void detach()
            {
                if (l)
                    l->r = r;
                if (r)
                    r->l = l;
                l = nullptr;
                r = nullptr;
            }

            void swap(Node &other)
            {
                Node* a = (l ? l : r);
                Node* b = (other.l ? other.l : other.r);
                detach();
                other.detach();
                if (a)
                    a->attach(&other);
                if (b)
                    b->attach(this);
            }
        };
    }

    using namespace details;

    template <typename T>
    class linked_ptr
    {
        template <typename U>
        friend class linked_ptr;

    private:
        mutable Node node;
        T* pointer;

    public:
        constexpr linked_ptr() noexcept : node(), pointer(nullptr) {}

        explicit linked_ptr(T* pointer) noexcept : node(), pointer(pointer) {}

        linked_ptr(linked_ptr const& other) : node(), pointer(other.get())
        {
            other.attach(*this);
        }

        template <typename U, typename = std::enable_if<std::is_convertible_v<U*, T*>>>
        explicit linked_ptr(U* pointer) : node(), pointer(pointer) {}

        template <typename U, typename = std::enable_if<std::is_convertible_v<U*, T*>>>
        linked_ptr(linked_ptr<U> const& other) : node(), pointer(other.get())
        {
            other.attach(*this);
        }

        ~linked_ptr()
        {
            destroy();
        }

        linked_ptr& operator=(linked_ptr const& other) noexcept
        {
            auto tmp(other);
            swap(tmp);
            return *this;
        }

        template <typename U, typename = std::enable_if<std::is_convertible_v<U*, T*>>>
        linked_ptr& operator=(linked_ptr<U> const& other)
        {
            auto tmp(other);
            swap(tmp);
            return *this;
        }

        template <typename U = T, typename = std::enable_if<std::is_convertible_v<U*, T*>>>
        void reset(U* new_pointer = nullptr)
        {
            destroy();
            pointer = new_pointer;
        }

        void swap(linked_ptr& other) noexcept
        {
            node.swap(other.node);
            std::swap(pointer, other.pointer);
        }

        T* get() const noexcept
        {
            return pointer;
        }

        bool unique() const noexcept
        {
            return !node.l && !node.r && pointer;
        }

        operator bool() const noexcept
        {
            return get();
        }

        T& operator*() const
        {
            return *get();
        }

        T* operator->() const
        {
            return get();
        }

    private:
        template <typename U = T, typename = std::enable_if<std::is_convertible_v<U*, T*>>>
        void attach(linked_ptr<U> const& copy) const noexcept
        {
            node.attach(&copy.node);
        }

        void detach() const noexcept
        {
            node.detach();
        }

        void destroy()
        {
            if (unique())
            {
                delete pointer;
            }
            pointer = nullptr;
            node.detach();
        }
    };


    template <typename T, typename U>
    inline bool operator==(linked_ptr<T> const& a, linked_ptr<U> const& b) noexcept
    {
        return a.get() == b.get();
    }

    template <typename T, typename U>
    inline bool operator!=(linked_ptr<T> const& a, linked_ptr<U> const& b) noexcept
    {
        return !(a == b);
    }

    template <typename T, typename U>
    inline bool operator<(linked_ptr<T> const& a, linked_ptr<U> const& b) noexcept
    {
        return a.get() < b.get();
    }

    template <typename T, typename U>
    inline bool operator>(linked_ptr<T> const& a, linked_ptr<U> const& b) noexcept
    {
        return b < a;
    }

    template <typename T, typename U>
    inline bool operator<=(linked_ptr<T> const& a, linked_ptr<U> const& b) noexcept
    {
        return a < b || a == b;
    }

    template <typename T, typename U>
    inline bool operator>=(linked_ptr<T> const& a, linked_ptr<U> const& b) noexcept
    {
        return (b <= a);
    }

    template <typename T>
    void swap(linked_ptr<T> &a, linked_ptr<T> &b) noexcept
    {
        a.swap(b);
    }
}

#endif
