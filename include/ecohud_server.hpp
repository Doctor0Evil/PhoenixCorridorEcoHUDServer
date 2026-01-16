#ifndef ECOHUD_SERVER_HPP
#define ECOHUD_SERVER_HPP

#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

#include "ecohud_csv.hpp"
#include "ecohud_json.hpp"

namespace ecohud
{
    class EcoHudServer
    {
    public:
        EcoHudServer(
            int port,
            std::vector<SmartCorridorRecord> smart_corridor_data,
            std::vector<CityKpiRecord> city_kpi_data
        )
        : port_(port)
        , smart_corridor_(std::move(smart_corridor_data))
        , city_kpi_(std::move(city_kpi_data))
        , running_(false)
        {}

        void start()
        {
            if (running_)
            {
                return;
            }

            running_ = true;
            server_thread_ = std::thread(&EcoHudServer::run, this);
        }

        void stop()
        {
            running_ = false;
            if (server_thread_.joinable())
            {
                server_thread_.join();
            }
        }

        ~EcoHudServer()
        {
            stop();
        }

    private:
        int port_;
        std::vector<SmartCorridorRecord> smart_corridor_;
        std::vector<CityKpiRecord> city_kpi_;
        std::atomic<bool> running_;
        std::thread server_thread_;

        static std::string http_response(
            const std::string& body,
            const std::string& content_type = "application/json"
        )
        {
            std::ostringstream oss;
            oss << "HTTP/1.1 200 OK\r\n";
            oss << "Content-Type: " << content_type << "\r\n";
            oss << "Content-Length: " << body.size() << "\r\n";
            oss << "Connection: close\r\n";
            oss << "\r\n";
            oss << body;
            return oss.str();
        }

        static std::string http_not_found()
        {
            const std::string body = "{\"error\":\"not_found\"}";
            return http_response(body);
        }

        static std::string http_bad_request(const std::string& msg)
        {
            const std::string body = "{\"error\":\"" + msg + "\"}";
            return http_response(body);
        }

        void run()
        {
            int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd < 0)
            {
                throw std::runtime_error("Failed to create socket");
            }

            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            {
                ::close(server_fd);
                throw std::runtime_error("Failed to set socket options");
            }

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(static_cast<uint16_t>(port_));

            if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
            {
                ::close(server_fd);
                throw std::runtime_error("Failed to bind socket");
            }

            if (listen(server_fd, 16) < 0)
            {
                ::close(server_fd);
                throw std::runtime_error("Failed to listen on socket");
            }

#ifdef ECOHUD_DEBUG_LOG
            std::printf("EcoHUD server listening on port %d\n", port_);
#endif

            while (running_)
            {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
                if (client_fd < 0)
                {
                    if (!running_)
                    {
                        break;
                    }
                    continue;
                }

                handle_client(client_fd);
                ::close(client_fd);
            }

            ::close(server_fd);
        }

        void handle_client(int client_fd)
        {
            char buffer[4096];
            const ssize_t bytes_read = ::recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read <= 0)
            {
                return;
            }
            buffer[bytes_read] = '\0';
            std::string request(buffer);

#ifdef ECOHUD_DEBUG_LOG
            std::printf("Request:\n%s\n", request.c_str());
#endif

            const std::string get_prefix = "GET ";
            const auto pos = request.find(get_prefix);
            if (pos == std::string::npos)
            {
                const std::string resp = http_bad_request("invalid_method");
                ::send(client_fd, resp.c_str(), resp.size(), 0);
                return;
            }

            const auto path_start = pos + get_prefix.size();
            const auto path_end = request.find(' ', path_start);
            if (path_end == std::string::npos)
            {
                const std::string resp = http_bad_request("invalid_request_line");
                ::send(client_fd, resp.c_str(), resp.size(), 0);
                return;
            }

            const std::string full_path = request.substr(path_start, path_end - path_start);

            std::string path = full_path;
            std::string query;
            const auto q_pos = full_path.find('?');
            if (q_pos != std::string::npos)
            {
                path = full_path.substr(0, q_pos);
                query = full_path.substr(q_pos + 1);
            }

            std::string response_body;
            bool found = false;

            if (path == "/corridor")
            {
                std::string corridor_id;
                const auto id_pos = query.find("id=");
                if (id_pos != std::string::npos)
                {
                    corridor_id = query.substr(id_pos + 3);
                }
                if (corridor_id.empty())
                {
                    const std::string resp = http_bad_request("missing_corridor_id");
                    ::send(client_fd, resp.c_str(), resp.size(), 0);
                    return;
                }

                response_body = to_json_corridor_tile(corridor_id, smart_corridor_);
                found = true;
            }
            else if (path == "/city")
            {
                std::string city_id;
                const auto id_pos = query.find("id=");
                if (id_pos != std::string::npos)
                {
                    city_id = query.substr(id_pos + 3);
                }
                if (city_id.empty())
                {
                    const std::string resp = http_bad_request("missing_city_id");
                    ::send(client_fd, resp.c_str(), resp.size(), 0);
                    return;
                }

                response_body = to_json_city_kpi_tile(city_id, city_kpi_);
                found = true;
            }

            if (!found)
            {
                const std::string resp = http_not_found();
                ::send(client_fd, resp.c_str(), resp.size(), 0);
            }
            else
            {
                const std::string resp = http_response(response_body);
                ::send(client_fd, resp.c_str(), resp.size(), 0);
            }
        }
    };

} // namespace ecohud

#endif // ECOHUD_SERVER_HPP
