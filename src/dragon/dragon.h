#ifndef DRAGON_DRAGON_H
#define DRAGON_DRAGON_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

namespace dragon
{
    namespace http
    {
        class Request
        {
        public:
            std::string method;
            std::string path;
            std::map<std::string, std::string> headers;
            std::string body;

            std::string param(const std::string &name) const
            {
                return "";
            }
        };

        class Response
        {
        public:
            int statusCode = 200;
            std::map<std::string, std::string> headers;
            std::string body;

            void send(const std::string &data)
            {
                body = data;
            }

            std::string toHttpResponse() const
            {
                std::stringstream ss;
                std::string statusMsg = (statusCode == 200) ? "OK" : "Not Found";
                ss << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n";
                ss << "Content-Type: text/plain\r\n";
                ss << "Content-Length: " << body.length() << "\r\n";
                ss << "Connection: close\r\n";
                ss << "\r\n";
                ss << body;
                return ss.str();
            }
        };

        class Router
        {
        public:
            void get(const std::string &path, std::function<void(Request &, Response &)> handler)
            {
                std::string key = "GET " + path;
                routes[key] = handler;
            }

            void post(const std::string &path, std::function<void(Request &, Response &)> handler)
            {
                std::string key = "POST " + path;
                routes[key] = handler;
            }

            void put(const std::string &path, std::function<void(Request &, Response &)> handler)
            {
                std::string key = "PUT " + path;
                routes[key] = handler;
            }

            void delete_(const std::string &path, std::function<void(Request &, Response &)> handler)
            {
                std::string key = "DELETE " + path;
                routes[key] = handler;
            }

            bool handleRequest(const std::string &method, const std::string &path, Request &req, Response &res) const
            {
                std::string key = method + " " + path;
                auto it = routes.find(key);
                if (it != routes.end())
                {
                    it->second(req, res);
                    return true;
                }
                return false;
            }

        private:
            std::map<std::string, std::function<void(Request &, Response &)>> routes;
        };

        class Server
        {
        public:
            Server(int port, const Router &router) : port_(port), router_(router), running_(false)
            {
                std::cout << "Server created on port " << port << std::endl;
            }

            void start()
            {
                running_ = true;
                std::cout << "Server started on port " << port_ << std::endl;
                std::cout << "Press Ctrl+C to stop the server..." << std::endl;

#ifdef _WIN32
                WSADATA wsaData;
                if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
                {
                    std::cerr << "WSAStartup failed" << std::endl;
                    return;
                }

                SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (listenSocket == INVALID_SOCKET)
                {
                    std::cerr << "socket failed" << std::endl;
                    WSACleanup();
                    return;
                }

                sockaddr_in serverAddr;
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                serverAddr.sin_port = htons(port_);

                if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
                {
                    std::cerr << "bind failed" << std::endl;
                    closesocket(listenSocket);
                    WSACleanup();
                    return;
                }

                if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
                {
                    std::cerr << "listen failed" << std::endl;
                    closesocket(listenSocket);
                    WSACleanup();
                    return;
                }

                std::cout << "Listening on http://127.0.0.1:" << port_ << std::endl;

                while (running_)
                {
                    sockaddr_in clientAddr;
                    int clientAddrLen = sizeof(clientAddr);
                    SOCKET clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

                    if (clientSocket == INVALID_SOCKET)
                    {
                        continue;
                    }

                    // Handle request in a new thread
                    std::thread(&Server::handleClient, this, clientSocket).detach();
                }

                closesocket(listenSocket);
                WSACleanup();
#endif
            }

            void stop()
            {
                running_ = false;
                std::cout << "Server stopped" << std::endl;
            }

        private:
            int port_;
            const Router &router_;
            bool running_;

            void handleClient(SOCKET clientSocket)
            {
                const size_t bufsize = 8192;
                std::string request;
                request.reserve(bufsize);
                char buffer[bufsize];
                int recvResult = recv(clientSocket, buffer, sizeof(buffer), 0);

                if (recvResult <= 0)
                {
                    closesocket(clientSocket);
                    return;
                }

                request.append(buffer, recvResult);

                // Read headers and body separator
                size_t header_end = request.find("\r\n\r\n");
                std::string headers_part;
                std::string body_part;
                if (header_end != std::string::npos)
                {
                    headers_part = request.substr(0, header_end);
                    body_part = request.substr(header_end + 4);
                }
                else
                {
                    // malformed request
                    closesocket(clientSocket);
                    return;
                }

                std::istringstream hs(headers_part);
                std::string request_line;
                std::getline(hs, request_line);
                if (!request_line.empty() && request_line.back() == '\r')
                    request_line.pop_back();
                std::istringstream rl(request_line);
                std::string method, path, httpVersion;
                rl >> method >> path >> httpVersion;

                Request req;
                req.method = method;
                req.path = path;

                // Parse headers
                std::string line;
                while (std::getline(hs, line))
                {
                    if (!line.empty() && line.back() == '\r')
                        line.pop_back();
                    if (line.empty())
                        break;
                    size_t colon = line.find(':');
                    if (colon != std::string::npos)
                    {
                        std::string name = line.substr(0, colon);
                        std::string value = line.substr(colon + 1);
                        // trim
                        while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
                            value.erase(0, 1);
                        req.headers[name] = value;
                    }
                }

                // If Content-Length present, ensure we've read full body
                size_t content_length = 0;
                auto it = req.headers.find("Content-Length");
                if (it != req.headers.end())
                {
                    try
                    {
                        content_length = std::stoul(it->second);
                    }
                    catch (...)
                    {
                        content_length = 0;
                    }
                }

                while (body_part.size() < content_length)
                {
                    int r = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (r <= 0)
                        break;
                    body_part.append(buffer, r);
                }

                req.body = body_part;

                Response res;

                // Try to handle the request
                if (!router_.handleRequest(method, path, req, res))
                {
                    res.statusCode = 404;
                    res.body = "Not Found";
                }

                std::string responseStr = res.toHttpResponse();
                send(clientSocket, responseStr.c_str(), (int)responseStr.length(), 0);

                closesocket(clientSocket);
            }
        };
    }
}

namespace Dragon
{
    inline void initialize(int argc, char *argv[])
    {
        std::cout << "Dragon framework initialized" << std::endl;
    }
}

#endif // DRAGON_DRAGON_H
