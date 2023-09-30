#pragma once

namespace Preset {
    namespace Xray {
        inline QStringList UtlsFingerPrint = {"", "chrome", "firefox", "edge", "safari", "360", "qq", "ios", "android", "random", "randomized"};
        inline QStringList ShadowsocksMethods = {"aes-128-gcm", "aes-256-gcm", "aes-192-gcm", "chacha20-ietf-poly1305", "xchacha20-ietf-poly1305",
                                                 "2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305",
                                                 "aes-128-ctr", "aes-192-ctr", "aes-256-ctr", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb",
                                                 "rc4", "rc4-md5", "bf-cfb", "chacha20", "chacha20-ietf", "xchacha20", "none"};
        // Note: chacha20-ietf: INonceSize is the size of the IETF-ChaCha20 nonce in bytes.
        //       chacha20: NonceSize is the size of the ChaCha20 nonce in bytes.
        //       2022-blake3-chacha20-poly1305 use XChaCha20-Poly1305 for UDP.
        // But who will use the junk?
        inline QStringList Flows = {"xtls-rprx-vision"};
        inline QStringList LogLevels = {"debug", "info", "warning", "error", "none"};
    } // namespace Xray

    namespace SingBox {
        inline QStringList VpnImplementation = {"gvisor", "system", "mixed"};
        inline QStringList DomainStrategy = {"", "ipv4_only", "ipv6_only", "prefer_ipv4", "prefer_ipv6"};
        inline QStringList UtlsFingerPrint = {"", "chrome", "firefox", "edge", "safari", "360", "qq", "ios", "android", "random", "randomized"};
        inline QStringList ShadowsocksMethods = {"2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305",
                                                 "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-ietf-poly1305", "xchacha20-ietf-poly1305",
                                                 "aes-128-ctr", "aes-192-ctr", "aes-256-ctr", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "rc4-md5",
                                                 "chacha20-ietf", "xchacha20", "none"};
        inline QStringList Flows = {"xtls-rprx-vision"};
        inline QStringList LogLevels = {"trace", "debug", "info", "warn", "error", "fatal", "panic"};
    } // namespace SingBox

    namespace Windows {
        inline QStringList system_proxy_format{"{ip}:{http_port}",
                                               "socks={ip}:{socks_port}",
                                               "http={ip}:{http_port};https={ip}:{http_port};ftp={ip}:{http_port};socks={ip}:{socks_port}",
                                               "http=http://{ip}:{http_port};https=http://{ip}:{http_port}"};
    } // namespace Windows
} // namespace Preset
