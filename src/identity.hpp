//
// Created by mbero-akoko on 2/13/26.
//

#ifndef NETWORKPROGRAMMING_IDENTITY_HPP
#define NETWORKPROGRAMMING_IDENTITY_HPP

#include <string_view>
#include <expected>
#include <variant>
#include <vector>
#include <system_error>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <generator> // C++23: The future of lazy iteration



namespace velocity_wave {
    enum class NetWorkError {
        ResolutionFailed,
        InvalidAddress,
        NoAddressFound,
        SytemError
    };



    using Endpoint = std::variant<sockaddr_in, sockaddr_in6>;

    class Resolver {
    public:

        using Result = std::expected<std::vector<Endpoint>, NetWorkError>;


        /**
         * @brief Resolves a hostname/port into a series of Endpoints.
         * @param host_name e.g., "localhost" or "127.0.0.1"
         * @param port e.g., "8080"
         * * In C++23, we can use std::generator to lazily yield results
         * from the underlying linked list returned by the OS.
         */
        static auto resolve(const std::string& host_name, const std::string_view& port) -> std::generator<Endpoint> {
            addrinfo hints {};
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            addrinfo* result = nullptr;

            if (getaddrinfo(host_name.data(), port.data(), &hints, &result) != 0) {
                co_return;
            }

            // RAII CLEANUP
            struct Cleanup {
                addrinfo* ptr;
                ~Cleanup(){ if (ptr) freeaddrinfo(ptr); }
            } cleanup{result};

            for (addrinfo* p = result; p != nullptr; p = p->ai_next) {
                if (p->ai_family == AF_INET) {
                    co_yield *reinterpret_cast<sockaddr_in*>( p -> ai_addr  );
                } else if ( p -> ai_family == AF_INET6 ) {
                    co_yield *reinterpret_cast<sockaddr_in6*>( p -> ai_addr  );
                }
            }
        }

        static auto to_string(const Endpoint& endpoint) -> std::string {
            char buffer[INET6_ADDRSTRLEN];
            const auto visitor = [&buffer]<typename addr_type>(addr_type && addr) -> std::string {
                using T = std::decay_t<addr_type>;
                if constexpr (std::is_same<T, sockaddr_in>::value) {
                    inet_ntop(AF_INET, &addr.sin_addr, buffer, sizeof(buffer));
                } else if constexpr (std::is_same_v<T, sockaddr_in6>) {
                    inet_ntop(AF_INET6, &addr.sin6_addr, buffer, sizeof(buffer));
                }

                return std::string{ buffer };
            };
            return std::visit(visitor, endpoint);
        };

     };
}

#endif //NETWORKPROGRAMMING_IDENTITY_HPP