#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

namespace NekoGui_fmt {
    void V2rayStreamSettings::BuildStreamSettingsSingBox(QJsonObject* outbound) {
        // https://sing-box.sagernet.org/configuration/shared/v2ray-transport

        if (network != "tcp") {
            QJsonObject transport{{"type", network}};
            if (network == "ws") {
                if (!host.isEmpty()) transport["headers"] = QJsonObject{{"Host", host}};
                // ws path & ed
                auto pathWithoutEd = SubStrBefore(path, "?ed=");
                if (!pathWithoutEd.isEmpty()) transport["path"] = pathWithoutEd;
                if (pathWithoutEd != path) {
                    auto ed = SubStrAfter(path, "?ed=").toInt();
                    if (ed > 0) {
                        transport["max_early_data"] = ed;
                        transport["early_data_header_name"] = "Sec-WebSocket-Protocol";
                    }
                }
                if (ws_early_data_length > 0) {
                    transport["max_early_data"] = ws_early_data_length;
                    transport["early_data_header_name"] = ws_early_data_name;
                }
            } else if (network == "http") {
                if (!path.isEmpty()) transport["path"] = path;
                if (!host.isEmpty()) transport["host"] = QList2QJsonArray(host.split(","));
            } else if (network == "grpc") {
                if (!path.isEmpty()) transport["service_name"] = path;
            }
            outbound->insert("transport", transport);
        } else if (header_type == "http") {
            // TCP + headerType
            QJsonObject transport{
                {"type", "http"},
                {"method", "GET"},
                {"path", path},
                {"headers", QJsonObject{{"Host", QList2QJsonArray(host.split(","))}}},
            };
            outbound->insert("transport", transport);
        }

        // 对应字段 tls
        if (security == "tls") {
            QJsonObject tls{{"enabled", true}};
            if (allow_insecure || NekoGui::dataStore->skip_cert) tls["insecure"] = true;
            if (!sni.trimmed().isEmpty()) tls["server_name"] = sni;
            if (!certificate.trimmed().isEmpty()) {
                tls["certificate"] = certificate.trimmed();
            }
            if (!alpn.trimmed().isEmpty()) {
                tls["alpn"] = QList2QJsonArray(alpn.split(","));
            }
            QString fp = utlsFingerprint;
            if (!reality_pbk.trimmed().isEmpty()) {
                tls["reality"] = QJsonObject{
                    {"enabled", true},
                    {"public_key", reality_pbk},
                    {"short_id", reality_sid.split(",")[0]},
                };
                if (fp.isEmpty()) fp = "random";
            }
            if (!fp.isEmpty()) {
                tls["utls"] = QJsonObject{
                    {"enabled", true},
                    {"fingerprint", fp},
                };
            }
            if (NekoGui::dataStore->enabled_ech) {
                tls["ech"] = QJsonObject{
                    {"enabled", true},
                    {"pq_signature_schemes_enabled", true},
                    {"dynamic_record_sizing_disabled", true},
                };
            }

            // disable_sni
            if (disable_sni) {
                if (reality_pbk.trimmed().isEmpty()) {
                    // 不使用 REALITY 时，使用 sing-box 提供的 disable SNI
                    // REALITY --> uTLS
                    // disable_sni -x> uTLS
                    tls["disable_sni"] = disable_sni;
                    tls["utls"] = QJsonObject{
                        {"enabled", false},
                    };
                } else {
                    tls["server_name"] = "";
                }
            }

            outbound->insert("tls", tls);
        }

        if (outbound->value("type").toString() == "vmess" || outbound->value("type").toString() == "vless") {
            outbound->insert("packet_encoding", packet_encoding);
        }
    }

    CoreObjOutboundBuildResult SocksHttpBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound;
        outbound["type"] = socks_http_type == type_HTTP ? "http" : "socks";
        if (socks_http_type == type_Socks4) outbound["version"] = "4";
        outbound["server"] = serverAddress;
        outbound["server_port"] = serverPort;

        if (!username.isEmpty() && !password.isEmpty()) {
            outbound["username"] = username;
            outbound["password"] = password;
        }

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult ShadowSocksBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{{"type", "shadowsocks"}};
        outbound["server"] = serverAddress;
        outbound["server_port"] = serverPort;
        outbound["method"] = method;
        outbound["password"] = password;

        if (uot != 0) {
            QJsonObject udp_over_tcp{
                {"enabled", true},
                {"version", uot},
            };
            outbound["udp_over_tcp"] = udp_over_tcp;
        } else {
            outbound["udp_over_tcp"] = false;
        }

        if (!plugin.trimmed().isEmpty()) {
            outbound["plugin"] = SubStrBefore(plugin, ";");
            outbound["plugin_opts"] = SubStrAfter(plugin, ";");
        }

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult VMessBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{
            {"type", "vmess"},
            {"server", serverAddress},
            {"server_port", serverPort},
            {"uuid", uuid.trimmed()},
            {"alter_id", aid},
            {"global_padding", true},
            {"authenticated_length", true},
            {"security", security},
        };

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult TrojanVLESSBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject outbound{
            {"type", proxy_type == proxy_VLESS ? "vless" : "trojan"},
            {"server", serverAddress},
            {"server_port", serverPort},
        };

        QJsonObject settings;
        if (proxy_type == proxy_VLESS) {
            if (flow.right(7) == "-udp443") {
                // 检查末尾是否包含"-udp443"，如果是，则删去
                flow.chop(7);
            } else if (flow == "none") {
                // 不使用 flow
                flow = "";
            }
            outbound["uuid"] = password.trimmed();
            outbound["flow"] = flow;
            // outbound["encryption"] = encryption; // Will be enabled in the feature
        } else {
            outbound["password"] = password;
        }

        stream->BuildStreamSettingsSingBox(&outbound);
        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult WireGuardBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        // reserved: string to int
        QJsonArray reservedJsonArray;
        if (!reserved.isEmpty()) {
            if (reserved.indexOf(',') != std::string::npos) {
                QStringList reservedList = reserved.split(",");
                QList<int> reservedIntList;

                foreach (const QString& str, reservedList) {
                    int number = str.toInt();
                    reservedIntList.append(number);
                }
                reservedJsonArray = QList2QJsonArray(reservedIntList);
            } else {
                QByteArray decoded = QByteArray::fromBase64(reserved.toUtf8());
                for (int i = 0; i < decoded.size(); i++) {
                    int value = static_cast<unsigned char>(decoded[i]);
                    reservedJsonArray.append(value);
                }
            }
        } else {
            reservedJsonArray = QJsonArray();
        }

        QJsonObject outbound{
            {"type", "wireguard"},
            {"server", serverAddress},
            {"server_port", serverPort},
            {"local_address", QList2QJsonArray(local_address.split(","))},
            {"private_key", private_key},
            {"peer_public_key", peer_public_key},
            {"pre_shared_key", pre_shared_key},
            {"reserved", reservedJsonArray},
            {"mtu", wireguard_mtu},
        };

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult QUICBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        QJsonObject coreTlsObj{
            {"enabled", true},
            {"disable_sni", disableSni},
            {"insecure", allowInsecure},
            {"certificate", caText.trimmed()},
            {"server_name", sni},
        };
        if (NekoGui::dataStore->enabled_ech) {
            coreTlsObj["ech"] = QJsonObject{
                {"enabled", true},
                {"pq_signature_schemes_enabled", true},
                {"dynamic_record_sizing_disabled", true},
            };
        }

        if (proxy_type == proxy_Hysteria2 && alpn.trimmed().isEmpty()) {
            coreTlsObj["alpn"] = "h3";
        } else {
            coreTlsObj["alpn"] = QList2QJsonArray(alpn.split(","));
        }

        QJsonObject outbound{
            {"server", serverAddress},
            {"server_port", serverPort},
            {"tls", coreTlsObj},
        };

        if (proxy_type == proxy_Hysteria) {
            outbound["type"] = "hysteria";
            outbound["obfs"] = obfsPassword;
            outbound["disable_mtu_discovery"] = disableMtuDiscovery;
            outbound["recv_window"] = streamReceiveWindow;
            outbound["recv_window_conn"] = connectionReceiveWindow;
            outbound["up_mbps"] = uploadMbps;
            outbound["down_mbps"] = downloadMbps;

            if (!hopPort.trimmed().isEmpty()) outbound["hop_ports"] = hopPort;
            if (authPayloadType == hysteria_auth_base64) outbound["auth"] = authPayload;
            if (authPayloadType == hysteria_auth_string) outbound["auth_str"] = authPayload;
        } else if (proxy_type == proxy_Hysteria2) {
            outbound["type"] = "hysteria2";
            outbound["password"] = password;
            outbound["up_mbps"] = uploadMbps;
            outbound["down_mbps"] = downloadMbps;
            if (!obfsPassword.isEmpty()) {
                outbound["obfs"] = QJsonObject{
                    {"type", "salamander"},
                    {"password", obfsPassword},
                };
            }
        } else if (proxy_type == proxy_TUIC) {
            outbound["type"] = "tuic";
            outbound["uuid"] = uuid;
            outbound["password"] = password;
            outbound["congestion_control"] = congestionControl;
            if (uos) {
                outbound["udp_over_stream"] = true;
            } else {
                outbound["udp_relay_mode"] = udpRelayMode;
            }
            outbound["zero_rtt_handshake"] = zeroRttHandshake;
            if (!heartbeat.trimmed().isEmpty()) outbound["heartbeat"] = heartbeat;
        }

        result.outbound = outbound;
        return result;
    }

    CoreObjOutboundBuildResult CustomBean::BuildCoreObjSingBox() {
        CoreObjOutboundBuildResult result;

        if (core == "internal") {
            result.outbound = QString2QJsonObject(config_simple);
        }

        return result;
    }
} // namespace NekoGui_fmt