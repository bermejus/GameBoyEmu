#pragma once

#include <experimental/coroutine>
#include <iostream>

struct Task
{
    struct promise_type
    {
        auto get_return_object() noexcept { return std::experimental::coroutine_handle<promise_type>::from_promise(*this); }

        auto initial_suspend() const noexcept { return std::experimental::suspend_always{}; }
        auto final_suspend() const noexcept { return std::experimental::suspend_always{}; }

        void return_void() const noexcept {}
        void unhandled_exception() const noexcept {}
    };

    Task() noexcept
        : _coro(nullptr)
    {}

    Task(std::experimental::coroutine_handle<promise_type> from) noexcept
        : _coro(from) {}

	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

    Task(Task&& t) noexcept
        : _coro(t._coro)
	{
		t._coro = nullptr;
	}

	Task& operator=(Task&& t) noexcept
	{
        if (std::addressof(t) != this)
        {
            if (_coro)
                _coro.destroy();
            _coro = t._coro;
        }
		return *this;
	}

    void operator()()
	{
		_coro.resume();
	}

    bool done() const noexcept
    {
        return _coro.done();
    }

    ~Task()
    {
        if (_coro && _coro.done())
            _coro.destroy();
    }

private:
    std::experimental::coroutine_handle<promise_type> _coro;
};