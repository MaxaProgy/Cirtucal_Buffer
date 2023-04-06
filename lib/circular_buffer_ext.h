#pragma once
#include "circular_buffer_common.h"
#include "iterator/random_access_iterator.h"

template <typename T, typename Alloc = std::allocator<T>>
class CircularBufferExt : public CircularBufferCommon<T, Alloc> {
   public:
    using typename CircularBufferCommon<T, Alloc>::allocator_type;
    using typename CircularBufferCommon<T, Alloc>::allocator_traits;

    using typename CircularBufferCommon<T, Alloc>::iterator;
    using typename CircularBufferCommon<T, Alloc>::const_iterator;

    using typename CircularBufferCommon<T, Alloc>::value_type;
    using typename CircularBufferCommon<T, Alloc>::reference;
    using typename CircularBufferCommon<T, Alloc>::pointer;
    using typename CircularBufferCommon<T, Alloc>::const_reference;
    using typename CircularBufferCommon<T, Alloc>::difference_type;
    using typename CircularBufferCommon<T, Alloc>::size_type;

    explicit CircularBufferExt(const Alloc& allocator = Alloc())
        : CircularBufferCommon<T, Alloc>(allocator) {}

    explicit CircularBufferExt(CircularBufferCommon<T, Alloc>::size_type n,
                               const Alloc& allocator = Alloc())
        : CircularBufferCommon<T, Alloc>(n, allocator) {}

    CircularBufferExt(CircularBufferCommon<T, Alloc>::size_type n,
                      CircularBufferCommon<T, Alloc>::value_type value,
                      const Alloc& allocator = Alloc())
        : CircularBufferCommon<T, Alloc>(n, value, allocator) {}

    CircularBufferExt(const CircularBufferExt<T, Alloc>& other)
        : CircularBufferCommon<T, Alloc>(other) {}

    template <typename LegacyInputIterator>
    CircularBufferExt(LegacyInputIterator i, LegacyInputIterator j,
                      const Alloc& allocator = Alloc())
        : CircularBufferCommon<T, Alloc>(i, j, allocator) {}

    CircularBufferExt(const std::initializer_list<value_type>& il,
                      const Alloc& allocator = Alloc())
        : CircularBufferCommon<T, Alloc>(il, allocator) {}

    ~CircularBufferExt() {
        clear();
        allocator_traits::deallocate(allocator_, container_begin_,
                                     capacity() + 1);
    }

    CircularBufferExt& operator=(const CircularBufferExt& other) {
        static_cast<CircularBufferCommon<T, Alloc>&>(*this).operator=(
            static_cast<CircularBufferCommon<T, Alloc>&>(other));
        return *this;
    }

    using CircularBufferCommon<T, Alloc>::begin;
    using CircularBufferCommon<T, Alloc>::end;
    using CircularBufferCommon<T, Alloc>::cbegin;
    using CircularBufferCommon<T, Alloc>::cend;
    using CircularBufferCommon<T, Alloc>::swap;
    using CircularBufferCommon<T, Alloc>::size;
    using CircularBufferCommon<T, Alloc>::capacity;
    using CircularBufferCommon<T, Alloc>::max_size;
    using CircularBufferCommon<T, Alloc>::empty;
    using CircularBufferCommon<T, Alloc>::reserve;
    using CircularBufferCommon<T, Alloc>::resize;
    using CircularBufferCommon<T, Alloc>::erase;
    using CircularBufferCommon<T, Alloc>::clear;
    using CircularBufferCommon<T, Alloc>::assign;
    using CircularBufferCommon<T, Alloc>::pop_back;
    using CircularBufferCommon<T, Alloc>::pop_front;
    using CircularBufferCommon<T, Alloc>::front;
    using CircularBufferCommon<T, Alloc>::back;
    using CircularBufferCommon<T, Alloc>::at;

    void swap(CircularBufferExt& other) {
        static_cast<CircularBufferCommon<T, Alloc>&>(*this).swap(
            static_cast<CircularBufferCommon<T, Alloc>&>(other));
    }

    void push_back(const T& value) {
        if (size() == capacity()) {
            expansion(capacity());
        }

        allocator_traits::construct(allocator_, data_end_, value);

        if (container_end_ == data_end_ + 1) {
            data_end_ = container_begin_;
        } else {
            ++data_end_;
        }
    }

    void push_front(const T& value) {
        if (size() == capacity()) {
            expansion(capacity());
        }

        pointer new_data_begin;

        if (container_begin_ == data_begin_) {
            new_data_begin = container_end_ - 1;
        } else {
            new_data_begin = data_begin_ - 1;
        }
        allocator_traits::construct(allocator_, new_data_begin, value);
        data_begin_ = new_data_begin;
    }

    iterator insert(const_iterator p, const_reference value) {
        if (std::addressof(*p) < container_begin_ ||
            std::addressof(*p) >= container_end_) {
            throw std::out_of_range("Iterator is out of bounds");
        }
        size_type index = p - begin();
        if (index > size()) {
            throw std::out_of_range("Iterator is out of bounds");
        }

        if (size() == capacity()) {
            expansion(capacity());
        }

        if (index == size()) {
            push_back(value);
            return --end();
        }
        if (index == 0) {
            push_front(value);
            return begin();
        }

        auto last = --end();
        auto it = begin() + index;

        allocator_traits::construct(allocator_, data_end_,
                                    std::move_if_noexcept(*last));

        if (container_end_ == data_end_ + 1) {
            data_end_ = container_begin_;
        } else {
            ++data_end_;
        }

        for (; last != it; --last) {
            *last = std::move_if_noexcept(*(last - 1));
        }
        *it = value;
        return it;
    }

    iterator insert(const_iterator p, size_type n, const_reference value) {
        if (std::addressof(*p) < container_begin_ ||
            std::addressof(*p) >= container_end_) {
            throw std::out_of_range("Iterator is out of bounds");
        }
        size_type index = p - begin();
        if (n == 0) {
            return begin() + index;
        }
        if (index > size()) {
            throw std::out_of_range("Iterator is out of bounds");
        }

        size_type target_capacity = capacity();
        while (target_capacity < size() + n) {
            target_capacity *= 2;
        }
        reserve(target_capacity);
        if (index == size()) {
            auto it = end();
            for (size_type i = 0; i < n; ++i) {
                push_back(value);
            }
            return it;
        }
        if (index == 0) {
            for (size_type i = 0; i < n; ++i) {
                push_front(value);
            }
            return begin();
        }

        if (data_end_ + n >= container_end_) {
            data_end_ = container_begin_ + ((data_end_ + n) - container_end_);
        } else {
            data_end_ += n;
        }

        auto to_insert = begin() + index;
        for (auto it = end() - n; it != to_insert; --it) {
            allocator_traits::construct(allocator_,
                                        std::addressof(*(it + n - 1)),
                                        std::move_if_noexcept(*(it - 1)));
        }
        for (size_type i = 0; i < n; ++i) {
            allocator_traits::construct(allocator_,
                                        std::addressof(to_insert[i]), value);
        }

        return to_insert;
    }

    template <typename LegacyInputIterator>
        requires std::input_iterator<LegacyInputIterator>
    iterator insert(const_iterator p, LegacyInputIterator i,
                    LegacyInputIterator j) {
        if (std::addressof(*p) < container_begin_ ||
            std::addressof(*p) >= container_end_) {
            throw std::out_of_range("Iterator is out of bounds");
        }
        size_type n = std::distance(i, j);
        size_type index = p - begin();
        if (n == 0) {
            return begin() + index;
        }
        if (index > size()) {
            throw std::out_of_range("Iterator is out of bounds");
        }

        size_type target_capacity = capacity();
        while (target_capacity < size() + n) {
            target_capacity *= 2;
        }
        reserve(target_capacity);

        if (index == size()) {
            auto it = end();
            for (; i != j; ++i) {
                push_back(*i);
            }
            return it;
        }
        if (index == 0) {
            for (; i != j; ++i) {
                push_front(*i);
            }
            return begin();
        }

        if (data_end_ + n >= container_end_) {
            data_end_ = container_begin_ + ((data_end_ + n) - container_end_);
        } else {
            data_end_ += n;
        }

        auto to_insert = begin() + index;
        for (auto it = end() - n; it != to_insert; --it) {
            allocator_traits::construct(allocator_,
                                        std::addressof(*(it + n - 1)),
                                        std::move_if_noexcept(*(it - 1)));
        }
        for (size_type k = 0; k < n; ++i, ++k) {
            allocator_traits::construct(allocator_,
                                        std::addressof(to_insert[k]), *i);
        }

        return to_insert;
    }

    iterator insert(const_iterator p,
                    const std::initializer_list<value_type>& il) {
        return insert(p, il.begin(), il.end());
    }

    bool operator==(const CircularBufferExt& other) const noexcept {
        return static_cast<const CircularBufferCommon<T, Alloc>&>(*this).
        operator==(static_cast<const CircularBufferCommon<T, Alloc>&>(other));
    }

    bool operator!=(const CircularBufferExt& other) const noexcept {
        return static_cast<const CircularBufferCommon<T, Alloc>&>(*this).
        operator!=(static_cast<const CircularBufferCommon<T, Alloc>&>(other));
    }

    reference operator[](size_type i) { return *(begin() + i); }

    const_reference operator[](size_type i) const { return *(cbegin() + i); }

   private:
    using CircularBufferCommon<T, Alloc>::container_begin_;
    using CircularBufferCommon<T, Alloc>::container_end_;
    using CircularBufferCommon<T, Alloc>::data_begin_;
    using CircularBufferCommon<T, Alloc>::data_end_;
    using CircularBufferCommon<T, Alloc>::allocator_;

    inline void expansion(size_type capacity) {
        if (!capacity) {
            reserve(1);
        }
        reserve(capacity * 2);
    }
};

template <typename T, typename Alloc>
void swap(CircularBufferExt<T, Alloc>& lhs, CircularBufferExt<T, Alloc>& rhs) {
    lhs.swap(rhs);
}