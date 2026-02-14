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

    template<typename T>
    concept IsValidEndpoint = std::is_same_v<T, sockaddr_in> || std::is_same_v<T, sockaddr_in6>;

    namespace usage {
        inline auto simple_resolve() -> void {
            std::string_view host = "google.com";
            std::string_view port = "443";

            std::cout << "Attempting to resolve " << host << "...\n";
            auto address_stream = Resolver::resolve(std::string(host), port);
            bool found { false };

            for (const auto& endpoint : address_stream) {
                found = true;

                std::cout << "Found potential endpoint " << Resolver::to_string(endpoint) <<"\n";
            }

            if (! found) {
                std::cerr << "Could not resolve host, please check your internet connection .\n";
            }

        };


        inline auto stress_test() -> void {
            // The targets: A mix of local and external
            const std::vector<std::pair<std::string_view, std::string_view>> targets = {
                {"google.com", "443"},
                {"localhost", "8080"},
                {"beej.us", "80"},
                {"github.com", "443"}
            };

            const int num_threads = 100; // Simulating 100 concurrent system requests
            const int iterations_per_thread = 10;

            std::atomic<size_t> total_resolved{0};

            std::latch start_gate { num_threads };
            std::vector<std::jthread> workers {};
            workers.reserve(num_threads);

            auto start_time = std::chrono::high_resolution_clock::now();


            for (std::size_t index = 0; index < num_threads ; ++index) {
                const auto task = [&, index](std::stop_token token) {
                    while (!token.stop_requested()) {
                        const auto target = targets[index % targets.size()];

                        start_gate.arrive_and_wait();

                        for (std::size_t j = 0; j < iterations_per_thread; ++j) {
                            const auto [host_name, port] = target;
                            auto stream = Resolver::resolve(std::string(host_name), port);
                            for (const auto endpoint: stream) {
                                total_resolved.fetch_add(1, std::memory_order::relaxed);
                            }
                        }
                    }
                };
                workers.emplace_back(task);
            }

            start_gate.wait();

            std::cout << "Total workers finished " << workers.size() << "\n";

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);


            std::cout << "--- Stress Test Results ---\n";
            std::cout << "Threads: " << num_threads << "\n";
            std::cout << "Total DNS Resolves: " << total_resolved.load() << "\n";
            std::cout << "Total Time: " << duration.count() << "ms\n";
            std::cout << "Avg per resolve: " << (double)duration.count() / (num_threads * iterations_per_thread) << "ms\n";




        }
    }
}

#endif //NETWORKPROGRAMMING_IDENTITY_HPP