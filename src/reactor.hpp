//
// Created by mbero-akoko on 2/15/26.
//

#ifndef NETWORKPROGRAMMING_REACTOR_HPP
#define NETWORKPROGRAMMING_REACTOR_HPP

#include <sys/epoll.h>
#include <bits/stdc++.h>
#include "socket_resource.hpp"
namespace velocity_wave {

    struct SocketAwaitable {
        int epoll_descriptor;
        int socket_descriptor;

        std::uint32_t events;

        auto await_ready() const noexcept -> bool { return false; }

        auto await_suspend(std::coroutine_handle<> handle)const noexcept -> void {
            epoll_event event_config{};

            event_config.events = events | EPOLLONESHOT | EPOLLET;
            event_config.data.ptr = handle.address();

            // There is more complexity in this line than meets the eye/
            // We use EPOLL_CTL_ADD for now but a more complex sytem would handle it
            // intelligenty by deciding between ADD and MOD
            epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, socket_descriptor, &event_config);

        }

        auto await_resume() const noexcept -> void {

        }
    };

    struct [[nodiscard]] Task{
        struct promise_type {
            Task get_return_object() {
                return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void unhandled_exception() { std::terminate(); }
            void return_void() {}
        };


        std::coroutine_handle<promise_type> coroutine_handle;

        explicit Task(std::coroutine_handle<promise_type> handle ): coroutine_handle(handle){}

        ~Task() { if (coroutine_handle) { coroutine_handle.destroy(); } }

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        Task(Task&& other) noexcept : coroutine_handle(std::exchange(other.coroutine_handle, nullptr)) {}

        /**
         * Book Keeping for move semantics
         * @param other
         * @return moved coroutine handle
         */
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                if (coroutine_handle) coroutine_handle.destroy();
                coroutine_handle = std::exchange(other.coroutine_handle, nullptr);
            }
            return *this;
        }
    };
}

#endif //NETWORKPROGRAMMING_REACTOR_HPP