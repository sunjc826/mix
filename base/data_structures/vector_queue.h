#pragma once
#include <base/types.h>
#include <base/result.h>

#include <vector>
#include <span>

namespace mix
{

template <typename T>
class VectorQueue
{
    union union_type { T value; };
    std::vector<union_type> buf;
    size_t head;
    size_t tail;
public:
    void enqueue(T item)
    {
        buf.push_back(item);
    }

    void enqueue(std::span<T> sp)
    {
        buf.resize(buf.size() + sp.size());
        std::copy(sp.begin(), sp.end(), buf.begin() + buf.tail);
    }

    bool empty() const
    {
        return head == tail;
    }

    size_t size() const
    {
        return tail - head;
    }

    Result<T> try_dequeue()
    {
        if (empty())
            return Result<T>::failure();
        else
            return Result<T>::success(dequeue());
    }

    T dequeue()
    {
        size_t const old_head = head;
        head++;
        if (empty())
            head = tail = 0;
        return buf[old_head];
    }

    std::span<T> dequeue(size_t size)
    {
        size_t const old_head = head;
        head += size;
        if (empty())
            head = tail = 0;
        return std::span<T>(reinterpret_cast<T *>(buf.begin() + old_head), size);
    }
};

}
