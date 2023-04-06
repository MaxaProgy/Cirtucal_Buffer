#pragma once

#include "iterator/random_access_iterator.h"

template <typename InputIterator, typename T, typename Alloc>
void copy_data(InputIterator start, InputIterator end, T* out,
               Alloc& allocator) {
    auto current = out;
    try {
        for (; start != end; ++start, ++current) {
            std::allocator_traits<Alloc>::construct(allocator, current, *start);
        }
    } catch (...) {
        for (; out != current; ++out) {
            std::allocator_traits<Alloc>::destroy(allocator, out);
        }
        throw;
    }
}

template <typename InputIterator, typename T, typename Alloc>
void move_data(InputIterator start, InputIterator end, T* out,
               Alloc& allocator) {
    auto current = out;
    try {
        for (; start != end; ++start, ++current) {
            std::allocator_traits<Alloc>::construct(
                allocator, current, std::move_if_noexcept(*start));
        }
    } catch (...) {
        for (; out != current; ++out) {
            std::allocator_traits<Alloc>::destroy(allocator, out);
        }
        throw;
    }
}

template <typename T, typename Alloc = std::allocator<T>>
class CircularBufferCommon
    : protected std::allocator_traits<Alloc>::template rebind_alloc<T> {
   public:
    using allocator_type =
        typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    using allocator_traits =
        typename std::allocator_traits<Alloc>::template rebind_traits<T>;

    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;

    using difference_type = iterator::difference_type;
    using size_type = std::size_t;

    static_assert(std::random_access_iterator<iterator>,
                  "my iterator isn't random access iterator");

    void swap(CircularBufferCommon& other) {
        if (this == &other) {
            return;
        }
        if constexpr (allocator_traits::propagate_on_container_swap::value) {
            std::swap(this->allocator_, other.allocator_);
            std::swap(container_begin_, other.container_begin_);
            std::swap(container_end_, other.container_end_);
            std::swap(data_begin_, other.data_begin_);
            std::swap(data_end_, other.data_end_);
            return;
        }

        const size_type this_old_size = this->size();
        const size_type this_old_capacity = this->capacity();

        const size_type other_old_size = other.size();
        const size_type other_old_capacity = other.capacity();

        pointer new_this_container_begin =
            allocator_traits::allocate(allocator_, other_old_capacity + 1);
        pointer new_other_container_begin;
        try {
            new_other_container_begin =
                allocator_traits::allocate(other.allocator_, this_old_size + 1);
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_this_container_begin,
                                         other_old_capacity + 1);
            throw;
        }

        try {
            move_data(this->begin(), this->end(), new_other_container_begin,
                      other.allocator_);
            move_data(other.begin(), other.end(), new_this_container_begin,
                      this->allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_this_container_begin,
                                         other_old_capacity + 1);
            allocator_traits::deallocate(other.allocator_,
                                         new_other_container_begin,
                                         this_old_capacity + 1);
            throw;
        }

        clear();
        other.clear();
        allocator_traits::deallocate(allocator_, this->container_begin_,
                                     this_old_capacity + 1);
        allocator_traits::deallocate(other.allocator_, other.container_begin_,
                                     other_old_capacity + 1);

        this->container_begin_ = new_this_container_begin;
        this->container_end_ =
            new_this_container_begin + other_old_capacity + 1;
        this->data_begin_ = this->container_begin_;
        this->data_end_ = this->data_begin_ + other_old_size;

        other.container_begin_ = new_other_container_begin;
        other.container_end_ =
            new_other_container_begin + this_old_capacity + 1;
        other.data_begin_ = other.container_begin_;
        other.data_end_ = other.data_begin_ + this_old_size;
    }

    iterator begin() noexcept {
        return iterator(data_begin_, container_begin_, container_end_,
                        data_begin_, data_end_);
    }

    iterator end() noexcept {
        return iterator(data_end_, container_begin_, container_end_,
                        data_begin_, data_end_);
    }
    const_iterator begin() const noexcept {
        return const_iterator(data_begin_, container_begin_, container_end_,
                              data_begin_, data_end_);
    }

    const_iterator end() const noexcept {
        return const_iterator(data_end_, container_begin_, container_end_,
                              data_begin_, data_end_);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(data_begin_, container_begin_, container_end_,
                              data_begin_, data_end_);
    }

    const_iterator cend() const noexcept {
        return const_iterator(data_end_, container_begin_, container_end_,
                              data_begin_, data_end_);
    }

    iterator erase(const_iterator q) {
        if (std::addressof(*q) < container_begin_ ||
            std::addressof(*q) >= container_end_) {
            throw std::out_of_range("Iterator is out of bounds");
        }
        size_type index = q - cbegin();
        if (index >= size()) {
            throw std::out_of_range("Iterator is out of bounds");
        }
        for (auto it = begin() + index; index < size() - 1; ++index, ++it) {
            *it = std::move_if_noexcept(*(it + 1));
        }

        if (container_begin_ == data_end_) {
            data_end_ = container_end_ - 1;
        } else {
            --data_end_;
        }

        allocator_traits::destroy(allocator_, data_end_);

        return begin() + index;
    }

    iterator erase(const_iterator q1, const_iterator q2) {
        if (std::addressof(*q1) < container_begin_ ||
            std::addressof(*q1) >= container_end_ ||
            std::addressof(*q2) < container_begin_ ||
            std::addressof(*q2) >= container_end_) {
            throw std::out_of_range("Iterator is out of bounds");
        }

        const size_type it_begin = q1 - cbegin();
        const size_type it_end = q2 - cbegin() - 1;
        const size_type n = it_end - it_begin + 1;

        if (it_begin >= size() || it_end >= size()) {
            throw std::out_of_range("Iterator is out of bounds");
        }

        for (auto it = q1; it != q2; ++it) {
            allocator_traits::destroy(allocator_, std::addressof(*it));
        }

        for (auto it = begin() + it_end + 1; it != end(); ++it) {
            *(it - n) = std::move_if_noexcept(*it);
        }

        data_end_ = std::addressof(*(end() - n));
        return begin() + it_begin;
    }

    void clear() noexcept {
        for (auto it = cbegin(); it != cend(); ++it) {
            allocator_traits::destroy(allocator_, std::addressof(*it));
        }
        data_begin_ = container_begin_;
        data_end_ = container_begin_;
    }

    void assign(size_type n, const_reference value) {
        pointer new_container_begin_ =
            allocator_traits::allocate(allocator_, n + 1);
        try {
            auto current = new_container_begin_;

            std::size_t i = 0;
            try {
                for (; i < n; ++i, ++current) {
                    std::allocator_traits<Alloc>::construct(allocator_, current,
                                                            value);
                }
            } catch (...) {
                for (; new_container_begin_ != current;
                     ++new_container_begin_) {
                    std::allocator_traits<Alloc>::destroy(allocator_,
                                                          new_container_begin_);
                }
                throw;
            }
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_container_begin_,
                                         n + 1);
            throw;
        }

        clear();
        allocator_traits::deallocate(allocator_, container_begin_,
                                     capacity() + 1);

        container_begin_ = new_container_begin_;
        container_end_ = container_begin_ + n + 1;
        data_begin_ = container_begin_;
        data_end_ = container_end_ - 1;
    }

    template <typename LegacyInputIterator>
        requires std::input_iterator<LegacyInputIterator>
    void assign(LegacyInputIterator i, LegacyInputIterator j) {
        size_type n = std::distance(i, j);

        pointer new_container_begin_ =
            allocator_traits::allocate(allocator_, n + 1);
        try {
            copy_data(i, j, new_container_begin_, allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_container_begin_,
                                         n + 1);
            throw;
        }

        clear();
        allocator_traits::deallocate(allocator_, container_begin_,
                                     capacity() + 1);

        container_begin_ = new_container_begin_;
        container_end_ = new_container_begin_ + n + 1;
        data_begin_ = container_begin_;
        data_end_ = container_end_ - 1;
    }

    void assign(const std::initializer_list<value_type>& il) {
        assign(il.begin(), il.end());
    }

    const_reference at(int n) const {
        if (n >= size()) {
            throw std::out_of_range("Iterator is out of bounds");
        }
        return *(cbegin() + n);
    }

    bool operator==(const CircularBufferCommon& other) const noexcept {
        if (this == &other) {
            return true;
        }

        return std::equal(cbegin(), cend(), other.cbegin(), other.cend());
    }

    bool operator!=(const CircularBufferCommon& other) const noexcept {
        return !(*this == other);
    }

    value_type pop_back() {
        if (empty()) {
            throw std::out_of_range(
                "Trying to pop_back() from an empty buffer");
        }

        if (container_begin_ == data_end_) {
            data_end_ = container_begin_ - 1;
        } else {
            --data_end_;
        }

        auto to_return = std::move(*data_end_);
        allocator_traits::destroy(allocator_, data_end_);

        return to_return;
    }

    value_type pop_front() {
        if (empty()) {
            throw std::out_of_range(
                "Trying to pop_back() from an empty buffer");
        }
        auto to_return = std::move(*data_begin_);
        allocator_traits::destroy(allocator_, data_begin_);

        if (data_begin_ + 1 == container_end_) {
            data_begin_ = container_begin_;
        } else {
            ++data_begin_;
        }

        return to_return;
    }

    size_type size() const noexcept { return std::distance(cbegin(), cend()); }

    size_type capacity() const noexcept {
        return std::distance(container_begin_, container_end_) - 1;
    }

    size_type max_size() const noexcept { return 123456789; }

    bool empty() const noexcept { return size() == 0; }

    reference front() {
        if (empty()) {
            throw std::out_of_range("Trying to get data from empty buffer");
        }
        return *data_begin_;
    }

    const_reference front() const {
        if (empty()) {
            throw std::out_of_range("Trying to get data from empty buffer");
        }
        return *data_begin_;
    }

    reference back() {
        if (container_begin_ == data_end_) {
            return *(container_end_ - 1);
        }

        return *(data_end_ - 1);
    }

    const_reference back() const {
        if (empty()) {
            throw std::out_of_range("Trying to get data from empty buffer");
        }
        if (container_begin_ == data_end_) {
            return *(container_end_ - 1);
        }

        return *(data_end_ - 1);
    }

    void reserve(size_type n) {
        if (capacity() >= n) {
            return;
        }
        auto new_container_begin =
            allocator_traits::allocate(allocator_, n + 1);
        try {
            move_data(begin(), end(), new_container_begin, allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_container_begin,
                                         n + 1);
            throw;
        }
        auto old_n = size();
        clear();
        allocator_traits::deallocate(allocator_, container_begin_,
                                     capacity() + 1);

        container_begin_ = new_container_begin;
        container_end_ = container_begin_ + n + 1;
        data_begin_ = container_begin_;
        data_end_ = data_begin_ + old_n;
    }

    void resize(size_type n, const value_type& value = value_type()) {
        if (n > capacity()) {
            reserve(n);
        }

        if (n == size()) {
            return;
        }

        if (n > size()) {
            size_type n_for_new_values = n - size();
            size_type i = 0;

            try {
                for (; i < n_for_new_values; ++i) {
                    allocator_traits::construct(allocator_, data_end_, value);

                    if (data_end_ + 1 == container_end_) {
                        data_end_ = container_begin_;
                    } else {
                        ++data_end_;
                    }
                }
            } catch (...) {
                for (size_type j = 0; j < i; ++j) {
                    if (container_begin_ == data_end_) {
                        data_end_ = container_end_ - 1;
                    } else {
                        --data_end_;
                    }

                    allocator_traits::destroy(allocator_, data_end_ + j);
                }
                throw;
            }
            return;
        }

        size_type n_for_del_values = size() - n;
        for (size_type i = 0; i < n_for_del_values; ++i) {
            if (container_begin_ == data_end_) {
                data_end_ = container_end_ - 1;
            } else {
                --data_end_;
            };

            allocator_traits::destroy(allocator_, data_end_);
        }
    }

   protected:
    pointer container_begin_;
    pointer container_end_;
    pointer data_begin_;
    pointer data_end_;

    explicit CircularBufferCommon(const Alloc& allocator = Alloc())
        : allocator_(allocator),
          container_begin_(allocator_traits::allocate(allocator_, 1)),
          container_end_(container_begin_ + 1),
          data_begin_(container_begin_),
          data_end_(container_begin_) {}

    CircularBufferCommon(const CircularBufferCommon& other)
        : allocator_(allocator_traits::select_on_container_copy_construction(
              other.allocator_)),
          container_begin_(
              allocator_traits::allocate(allocator_, other.size() + 1)),
          container_end_(container_begin_ + other.size() + 1),
          data_begin_(container_begin_),
          data_end_(data_begin_ + other.size()) {
        try {
            copy_data(other.begin(), other.end(), container_begin_, allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, container_begin_,
                                         capacity() + 1);
            throw;
        }
    }

    explicit CircularBufferCommon(size_type size,
                                  const Alloc& allocator = Alloc())
        : allocator_(allocator),
          container_begin_(allocator_traits::allocate(allocator_, size + 1)),
          container_end_(container_begin_ + size + 1),
          data_begin_(container_begin_),
          data_end_(container_begin_) {}

    CircularBufferCommon(size_type size, const_reference value,
                         const Alloc& allocator = Alloc())
        : allocator_(allocator),
          container_begin_(allocator_traits::allocate(allocator_, size + 1)),
          container_end_(container_begin_ + size + 1),
          data_begin_(container_begin_),
          data_end_(container_end_ - 1) {
        size_type current = 0;
        try {
            for (; current < size; ++current) {
                allocator_traits::construct(allocator_, data_begin_ + current,
                                            value);
            }
        } catch (...) {
            for (size_type i = 0; i < current; ++i) {
                allocator_traits::destroy(allocator_, data_begin_ + i);
            }
            allocator_traits::deallocate(allocator_, container_begin_,
                                         size + 1);
            throw;
        }
    }

    template <typename LegacyInputIterator>
        requires std::input_iterator<LegacyInputIterator>

    CircularBufferCommon(LegacyInputIterator i, LegacyInputIterator j,
                         const Alloc& allocator = Alloc())
        : allocator_(allocator),
          container_begin_(
              allocator_traits::allocate(allocator_, std::distance(i, j) + 1)),
          container_end_(container_begin_ + std::distance(i, j) + 1),
          data_begin_(container_begin_),
          data_end_(container_end_ - 1) {
        try {
            copy_data(i, j, data_begin_, allocator_);
        } catch (...) {
            allocator_traits::deallocate(
                allocator_, container_begin_,
                std::distance(container_begin_, container_end_));
            throw;
        }
    }
    CircularBufferCommon(const std::initializer_list<value_type>& il,
                         const Alloc& allocator = Alloc())
        : allocator_(allocator),
          container_begin_(
              allocator_traits::allocate(allocator_, il.size() + 1)),
          container_end_(container_begin_ + il.size() + 1),
          data_begin_(container_begin_),
          data_end_(container_end_ - 1) {
        try {
            copy_data(il.begin(), il.end(), data_begin_, allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, container_begin_,
                                         il.size() + 1);
            throw;
        }
    }

    CircularBufferCommon& operator=(
        const CircularBufferCommon& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if constexpr (allocator_traits::propagate_on_container_copy_assignment::
                          value) {
            allocator_type new_allocator = other;

            auto new_containter_begin =
                allocator_traits::allocate(new_allocator, other.size() + 1);
            try {
                copy_data(other.begin(), other.end(), new_containter_begin,
                          new_allocator);
            } catch (...) {
                allocator_traits::deallocate(
                    new_allocator, new_containter_begin, other.size() + 1);
                throw;
            }

            clear();
            allocator_traits::deallocate(allocator_, container_begin_,
                                         capacity() + 1);

            allocator_ = std::move(new_allocator);
            container_begin_ = new_containter_begin;
            container_end_ = new_containter_begin + other.size() + 1;
            data_begin_ = container_begin_;
            data_end_ = container_end_ - 1;

            return *this;
        }

        pointer new_containter_begin =
            allocator_traits::allocate(allocator_, other.size() + 1);
        try {
            copy_data(other.begin(), other.end(), new_containter_begin,
                      allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_containter_begin,
                                         other.size() + 1);
            throw;
        }

        clear();
        allocator_traits::deallocate(
            allocator_, container_begin_,
            std::distance(container_begin_, container_end_));

        container_begin_ = new_containter_begin;
        container_end_ = new_containter_begin + other.size() + 1;
        data_begin_ = container_begin_;
        data_end_ = container_end_ - 1;

        return *this;
    }

    CircularBufferCommon& operator=(
        const std::initializer_list<value_type>& list) {
        pointer new_containter_begin =
            allocator_traits::allocate(allocator_, list.size() + 1);

        try {
            copy_data(list.begin(), list.end(), new_containter_begin,
                      allocator_);
        } catch (...) {
            allocator_traits::deallocate(allocator_, new_containter_begin,
                                         list.size() + 1);
            throw;
        }

        clear();
        allocator_traits::deallocate(allocator_, container_begin_,
                                     capacity() + 1);

        container_begin_ = new_containter_begin;
        container_end_ = container_begin_ + list.size() + 1;
        data_begin_ = container_begin_;
        data_end_ = container_end_ - 1;

        return *this;
    }

    allocator_type allocator_;
};
