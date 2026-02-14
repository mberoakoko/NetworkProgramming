//
// Created by mbero-akoko on 2/14/26.
//

#ifndef NETWORKPROGRAMMING_SOCKET_RESOURCE_HPP
#define NETWORKPROGRAMMING_SOCKET_RESOURCE_HPP


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
}
#endif //NETWORKPROGRAMMING_SOCKET_RESOURCE_HPP