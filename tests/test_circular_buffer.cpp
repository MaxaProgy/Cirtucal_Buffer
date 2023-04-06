#include <gtest/gtest.h>

#include <string>

#include "lib/circular_buffer.h"

TEST(PUSH_TEST, ALTERNATING_PUSH) {
    CircularBuffer<int> cb(8);
    const CircularBuffer<int> result({21, 15, 3, 1, 0, 6, 10, 28});
    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    ASSERT_TRUE(cb == result);
}

TEST(PUSH_TEST, PUSHING_COMPLICATED_OBJECTS_WITH_ERASE) {
    CircularBuffer<std::string> cb = {"Max", "Maxa"};
    const CircularBuffer<std::string> result = {
        "Marina",
        "Max",
        "Maxa",
        "Misha",
    };
    cb.reserve(4);
    cb.push_front("Marina");
    cb.push_back("Misha");

    ASSERT_TRUE(cb == result);
}

TEST(POP_TEST, SIMPLE_POP_TEST) {
    CircularBuffer<int> cb(8);  // 21, 15, 3, 1, 0, 6, 10, 28
    const CircularBuffer<int> result = {3, 1, 0, 6};
    cb.push_back(0);
    cb.push_back(6);
    cb.push_back(10);
    cb.push_back(28);
    cb.push_front(1);
    cb.push_front(3);
    cb.push_front(15);
    cb.push_front(21);

    cb.pop_back();
    cb.pop_front();
    cb.pop_back();
    cb.pop_front();

    ASSERT_TRUE(cb == result);
}

TEST(POP_TEST, POP_FROM_EMPTY) {
    CircularBuffer<int> cb;
    try {
        cb.pop_back();
        FAIL();
    } catch (...) {
    }

    try {
        cb.pop_front();
        FAIL();
    } catch (...) {
    }

    SUCCEED();
}

TEST(ERASE_TEST, ERASE_ONE_ELEMENT) {
    CircularBuffer<int> cb = {21, 15, 3, 1, 0, 6, 10, 28};

    cb.erase(cb.cbegin() + 1);

    ASSERT_TRUE(cb == CircularBuffer<int>({21, 3, 1, 0, 6, 10, 28}));
}

TEST(ERASE_TEST, ERASE_SEQUENCE) {
    CircularBuffer<int> cb(8);
    const CircularBuffer<int> result({21, 15, 3, 1, 0, 6, 10, 28});

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    cb.erase(cb.cbegin() + 2, cb.cend() - 1);

    ASSERT_TRUE(cb == CircularBuffer<int>({21, 15, 28}));
}

TEST(AT_TEST, AT_TAKE_MORE) {
    CircularBuffer<int> cb = {21, 15, 3, 1, 0, 6, 10, 28};

    try {
        cb.at(666);
        FAIL();
    } catch (const std::out_of_range& err) {
        SUCCEED();
    }
}

TEST(ASSIGN_TEST, N_VALUES) {
    CircularBuffer<int> cb;
    cb.assign(3, 666);

    ASSERT_TRUE(cb == CircularBuffer<int>({666, 666, 666}));
}

TEST(ASSIGN_TEST, ASSIGN_ITERATOR) {
    CircularBuffer<int> cb;
    std::vector<int> v = {21, 15, 3, 1, 0, 6, 10, 28};

    cb.assign(v.begin() + 3, v.end());

    ASSERT_TRUE(cb == CircularBuffer<int>({1, 0, 6, 10, 28}));
}

TEST(ASSIGN_TEST, INITIALIZER_LIST) {
    CircularBuffer<int> cb = {1, 2, 3, 4, 5, 6, 7, 8};

    cb.assign({21, 15, 3, 1, 0, 6, 10, 28});

    ASSERT_TRUE(cb == CircularBuffer<int>({21, 15, 3, 1, 0, 6, 10, 28}));
}

TEST(RESERVE_TEST, SIMPLE_TEST) {
    CircularBuffer<int> cb = {21, 15, 3, 1, 0, 6, 10, 28};
    CircularBuffer<int> copy_cb = cb;

    cb.reserve(10);
    ASSERT_EQ(cb.capacity(), 10);
    ASSERT_TRUE(cb == copy_cb);
}

TEST(RESIZE_TEST, EXPAND) {
    CircularBuffer<int> cb = {21, 15, 3, 1, 0, 6, 10, 28};
    cb.resize(10, 666);

    ASSERT_TRUE(cb ==
                CircularBuffer<int>({21, 15, 3, 1, 0, 6, 10, 28, 666, 666}));
}

TEST(RESIZE_TEST, SHRINK) {
    CircularBuffer<int> cb = {21, 15, 3, 1, 0, 6, 10, 28};
    cb.resize(5);

    ASSERT_TRUE(cb == CircularBuffer<int>({21, 15, 3, 1, 0}));
}

TEST(INSERT_TEST, INSERT_VALUE) {
    CircularBuffer<int> cb(8);  //  {21, 15, 3, 1, 0, 6, 10, 28}

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    cb.insert(cb.begin() + 3, 666);

    ASSERT_TRUE(cb == CircularBuffer<int>({21, 15, 3, 666, 1, 0, 6, 10, 28}));
}

TEST(INSERT_TEST, INSERT_N_VALUES) {
    CircularBuffer<int> cb(8);  //  {21, 15, 3, 1, 0, 6, 10, 28}

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    cb.insert(cb.begin() + 2, 2, 666);

    ASSERT_TRUE(cb ==
                CircularBuffer<int>({21, 15, 666, 666, 3, 1, 0, 6, 10, 28}));
}

TEST(INSERT_TEST, INSERT_ITERATORS) {
    CircularBuffer<int> cb(8);  // {21, 15, 3, 1, 0, 6, 10, 28}
    std::vector<int> v = {666, 667, 668};

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    cb.insert(cb.begin() + 2, v.begin(), v.end());

    ASSERT_TRUE(
        cb == CircularBuffer<int>({21, 15, 666, 667, 668, 3, 1, 0, 6, 10, 28}));
}

TEST(INSERT_TEST, INSERT_INITIALIZER_LIST) {
    CircularBuffer<int> cb(8);  // {21, 15, 3, 1, 0, 6, 10, 28}

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    cb.insert(cb.begin() + 2, {666, 667, 668});

    ASSERT_TRUE(
        cb == CircularBuffer<int>({21, 15, 666, 667, 668, 3, 1, 0, 6, 10, 28}));
}

TEST(FRONT, SIMPLE_TEST) {
    CircularBuffer<int> cb(8);  // {21, 15, 3, 1, 0, 6, 10, 28}

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }
    ASSERT_TRUE(cb.front() == 21);
}

TEST(BACK, SIMPLE_TEST) {
    CircularBuffer<int> cb(8);  // {21, 15, 3, 1, 0, 6, 10, 28}

    int j = 0;
    for (int i = 0; i < 8; ++i) {
        j += i;
        cb[i] = j;

        if (cb[i] % 2 == 0) {
            cb.push_back(cb[i]);
        } else {
            cb.push_front(cb[i]);
        }
    }

    ASSERT_TRUE(cb.back() == 28);
};