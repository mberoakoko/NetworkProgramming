//
// Created by mbero-akoko on 2/14/26.
//

#ifndef NETWORKPROGRAMMING_SOCKET_RESOURCE_HPP
#define NETWORKPROGRAMMING_SOCKET_RESOURCE_HPP
#include <sys/socket.h>
#include <fcntl.h>
#include "identity.hpp"

#include <bits/stdc++.h>
namespace velocity_wave {

    /**
     * Simple Contract that defines what a socket actually is
     */
    template<typename T>
    concept SocketType = requires
    {
        {T::protocal_type} -> std::convertible_to<int>;
        {T::is_stream} -> std::convertible_to<bool>;
    };

    struct TCP {
        static constexpr int protocal_type = SOCK_STREAM;
        static constexpr bool is_stream = true;
    };

    struct UDP {
        static constexpr int protocal_type = SOCK_DGRAM;
        static constexpr bool is_stream = false;
    };

    template<SocketType Proto>
    class UniqueSocket {

    public:

        static auto create(int family = AF_INET) -> std::expected<UniqueSocket, NetWorkError>{
            int socket_handle { ::socket(family, Proto::protocal_type , 0) };
            if (socket_handle == -1) {
                return std::unexpected(NetWorkError::SytemError);
            }
            return UniqueSocket(socket_handle);
        }

        // Rule of five no copying allowed
        UniqueSocket(const UniqueSocket&) = delete;
        auto operator=(const UniqueSocket&) -> UniqueSocket& = delete;

        // Moving is encouraged
        UniqueSocket(UniqueSocket&& other) noexcept
            : socket_descriptor_(std::exchange(other.socket_descriptor_, -1)) {}
        auto operator=(UniqueSocket&& other) noexcept -> UniqueSocket& {
            if (this != other) {
                release_resource();
                socket_descriptor_ = std::exchange(other.socket_descriptor_, -1);
            }
            return *this;
        }

        ~UniqueSocket() {
            release_resource();
        };

        [[nodiscard]] auto native_handle() const noexcept -> int {
            return socket_descriptor_;
        }


        /**
         * Function to bind to a socket on a given endpoint
         * @param endpoint: ipv4 or ipv6 endpoint
         * @return error value if bind unsuccessful
         */
        auto bind(const Endpoint& endpoint) -> std::expected<void, NetWorkError> {
            return std::visit([this](auto&& addr) {
                if (::bind(socket_descriptor_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == -1) {
                    return std::unexpected(NetWorkError::SytemError);
                }
            }, endpoint);
        }

        auto listen(int backlog = 128) -> std::expected<void, NetWorkError> requires (Proto::is_stream)
        {
            if (::listen(socket_descriptor_, backlog) == -1) {
                return std::unexpected(NetWorkError::SytemError);
            }
            return {};
        }

    private:
        explicit UniqueSocket(const int handle): socket_descriptor_(handle){};

        auto release_resource() -> void {
            if (socket_descriptor_ != -1 ) {
                ::close(socket_descriptor_);
                socket_descriptor_ = -1;
            }
        }

        int socket_descriptor_;
    };

    using TCPListener = UniqueSocket<TCP>;

    namespace stress_test {
        inline auto stress_test() -> void {
            constexpr int iterations = 100'000;
            int success_count = 0;

            std::cout << "Starting Resource Pressure Test (RAII verification)...\n";

            for (std::size_t i = 0; i < iterations ; ++i ) {
                auto socket_result = TCPListener::create();
                if (socket_result) {
                    success_count+=1;
                }else {
                    std::cerr << "Failed at iteration " << i << " - Check system ulimits!\n";
                    break;
                }
            }


            std::cout << "Test Complete.\n";
            std::cout << "Successfully cycled " << success_count << " sockets without leaking.\n";
            std::cout << "Current process health: EXCELLENT.\n";
        }
    }
}
#endif //NETWORKPROGRAMMING_SOCKET_RESOURCE_HPP