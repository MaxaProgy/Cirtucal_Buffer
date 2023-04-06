#pragma once
#include <iterator>

template <typename T>
class Iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::random_access_iterator_tag;

   private:
    pointer current_;
    pointer container_begin_;
    pointer container_end_;
    pointer data_begin_;
    pointer data_end_;

   public:
    Iterator() = default;

    Iterator(pointer current, pointer begin, pointer end, pointer data_begin,
             pointer data_end)
        : current_(current),
          container_begin_(begin),
          container_end_(end),
          data_begin_(data_begin),
          data_end_(data_end) {}

    ~Iterator() noexcept = default;

    operator Iterator<const T>() {
        return Iterator<const T>(current_, container_begin_, container_end_,
                                 data_begin_, data_end_);
    }

    reference operator*() const noexcept { return *current_; }

    reference operator->() const noexcept { return current_; }

    reference operator[](std::size_t n) const noexcept {
        return *this->operator+(n);
    }

    Iterator& operator++() noexcept {
        if (container_begin_ + 1 == container_end_) return *this;

        if (current_ + 1 != container_end_) {
            current_++;
        } else {
            current_ = container_begin_;
        }
        return *this;
    }

    Iterator operator++(int) const noexcept {
        if (container_begin_ + 1 == container_end_) return *this;

        const auto old = current_;
        if (current_ + 1 != container_end_) {
            ++current_;
        } else {
            current_ = container_begin_;
        }

        return Iterator(old, container_begin_, container_end_, data_begin_,
                        data_end_);
    }

    Iterator& operator--() noexcept {
        if (container_begin_ + 1 == container_end_) return *this;

        if (current_ != container_begin_) {
            --current_;
        } else {
            current_ = container_end_ - 1;
        }

        return *this;
    }
    Iterator operator--(int) const noexcept {
        if (container_begin_ + 1 == container_end_) return *this;

        const auto old = current_;
        if (current_ != container_begin_) {
            --current_;
        } else {
            current_ = container_end_ - 1;
        }

        return Iterator(old, container_begin_, container_end_, data_begin_,
                        data_end_);
    }

    Iterator operator+(int n) const noexcept {
        if (container_begin_ + 1 == container_end_) return *this;

        if (n == 0) {
            return *this;
        }
        if (n < 0) {
            return this->operator-(-n);
        }
        const auto to_end = std::distance(current_, container_end_);

        if (n < to_end) {
            return Iterator(current_ + n, container_begin_, container_end_,
                            data_begin_, data_end_);
        }
        return Iterator(
            container_begin_ +
                (n - to_end) % std::distance(container_begin_, container_end_),
            container_begin_, container_end_, data_begin_, data_end_);
    }

    Iterator& operator+=(int n) noexcept {
        if (container_begin_ + 1 == container_end_) {
            return *this;
        };

        if (n == 0) return *this;

        if (n < 0) return this->operator-=(-n);

        const auto to_end = std::distance(current_, container_end_);
        if (n < to_end) {
            current_ += n;
            return *this;
        }

        current_ =
            container_begin_ +
            (n - to_end) % std::distance(container_begin_, container_end_);
        return *this;
    }

    Iterator operator-(int n) const noexcept {
        if (container_begin_ + 1 == container_end_) return *this;

        if (n == 0) {
            return *this;
        }
        if (n < 0) return this->operator+(-n);

        const auto to_begin = std::distance(container_begin_, current_);

        if (n <= to_begin) {
            return Iterator(current_ - n, container_begin_, container_end_,
                            data_begin_, data_end_);
        }
        return Iterator(
            container_end_ - (n - to_begin) % std::distance(container_begin_,
                                                            container_end_),
            container_begin_, container_end_, data_begin_, data_end_);
    }

    Iterator& operator-=(int n) noexcept {
        if (container_begin_ + 1 == container_end_) {
            return *this;
        }

        if (n <= 0) {
            return this->operator+=(-n);
        }
        const auto to_begin = std::distance(container_begin_, current_);
        if (n <= to_begin) {
            return Iterator(current_ - n, container_begin_, container_end_,
                            data_begin_, data_end_);
        }
        return Iterator(
            container_end_ - (n - to_begin) % std::distance(container_begin_,
                                                            container_end_),
            container_begin_, container_end_, data_begin_, data_end_);
    }

    difference_type operator-(const Iterator<T>& rhs) const {
        if (container_begin_ != rhs.container_begin_ ||
            container_end_ != rhs.container_end_)
            throw std::out_of_range("Iterator is out of bounds");

        if (current_ >= data_begin_ && rhs.current_ >= data_begin_)
            return std::distance(rhs.current_, current_);

        if (current_ >= data_begin_ && rhs.current_ < data_begin_)
            return std::distance(container_begin_, rhs.current_) -
                   std::distance(current_, container_end_);

        if (rhs.current_ > data_end_)
            return std::distance(rhs.current_, container_end_) +
                   std::distance(container_begin_, current_);

        return std::distance(rhs.current_, current_);
    }

    bool operator==(const Iterator& rhs) const noexcept {
        return current_ == rhs.current_;
    }

    bool operator!=(const Iterator& rhs) const noexcept {
        return !(this->operator==(rhs));
    }
    bool operator>(const Iterator& rhs) const noexcept {
        return (this->operator!=(rhs) && this->operator-(rhs) > 0);
    }

    bool operator>=(const Iterator& rhs) const noexcept {
        return !(this->operator<(rhs));
    }

    bool operator<(const Iterator& rhs) const noexcept {
        return (this->operator!=(rhs) && this->operator-(rhs) < 0);
    }

    bool operator<=(const Iterator& rhs) const noexcept {
        return !(this->operator>(rhs));
    }
};

template <typename T>
Iterator<T> operator+(int n, const Iterator<T>& rhs) {
    return rhs + n;
}
