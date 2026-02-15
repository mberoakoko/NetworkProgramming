//
// Created by mbero-akoko on 2/15/26.
//

#ifndef NETWORKPROGRAMMING_AUDIO_PIPELINE_HPP
#define NETWORKPROGRAMMING_AUDIO_PIPELINE_HPP

#include <bits/stdc++.h>
#include <sys/sendfile.h>
#include "socket_resource.hpp"

namespace velocity_wave {
    struct WavHeader {
        char chunk_id[4];     // "RIFF"
        std::uint32_t chunk_size;
        char format[4];       // "WAVE"
        char subchunk1_id[4]; // "fmt "
        std::uint32_t subchunk1_size;
        std::uint16_t audio_format;
        std::uint16_t num_channels;
        std::uint32_t sample_rate;
        std::uint32_t byte_rate;
        std::uint16_t block_align;
        std::uint16_t bits_per_sample;
        char subchunk2_id[4]; // "data"
        std::uint32_t subchunk2_size;
    };

    class AudioPipeline {

    public:
        static auto parse_header(std::string_view file_path) -> std::expected<WavHeader, NetWorkError> {
            std::ifstream file(file_path.data(), std::ios::binary);

            if (!file){ return std::unexpected(NetWorkError::SytemError); }

            WavHeader header{};

            if ( !file.read(reinterpret_cast<char *>(&header), sizeof(WavHeader)) ) {
                return std::unexpected<NetWorkError>(NetWorkError::InvalidAddress);
            }

            bool is_invalid_header_data {
                    std::string_view(header.chunk_id, 4) != "RIFF" ||
                    std::string_view(header.chunk_id, 4) != "WAVE"
            };

            if (is_invalid_header_data) {
                return std::unexpected(NetWorkError::InvalidAddress);
            }

            return header;
        }
    };

    template<SocketType Proto>
    static auto stream_to_socket(
        int file_descriptor,
        UniqueSocket<Proto> client_socket,
        std::size_t offset,
        std::size_t count) -> std::expected<std::size_t, NetWorkError> {

        const auto out_offset = static_cast<off_t>(offset);
        ssize_t sent = ::sendfile(client_socket.native_handle(), file_descriptor, &out_offset, count);

        if (sent == -1) { return std::unexpected(NetWorkError::SytemError); }

        return static_cast<std::size_t>(sent);
    }
}

#endif //NETWORKPROGRAMMING_AUDIO_PIPELINE_HPP