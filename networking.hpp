#pragma once
#include "include/asio.hpp"
#include "include/asio/ts/buffer.hpp"
#include "include/asio/ts/internet.hpp"

class Networking {
public:
  struct Endpoint {
    asio::ip::tcp::endpoint asioEndpoint;
    std::string domain, path;
  };
  struct Peer {
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::socket socket;
    asio::error_code asioErr;
    Peer(Networking& networking, std::string ip, int port) :
      socket(networking.GetAsioContext()) {
      endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(ip), port);
      socket.connect(endpoint, asioErr);
      if (asioErr)
        std::cout << "Could not connect to peer: " << asioErr.message();
      else
        std::cout << "Connected to peer.\n";
    }
    // Returns the response or an empty string if the function fails
    std::string Send(std::string data) {
      if (!socket.is_open()) {
        std::cout << "Could not send data to peer: socket not open.\n";
        return "";
      }
      socket.write_some(asio::buffer(data.data(), data.size()), asioErr);
      // Wait for the response
      socket.wait(socket.wait_read);
      int bytes = socket.available();
      std::string response;
      if (bytes > 0) {
        response.resize(bytes);
        socket.read_some(asio::buffer(response.data(), bytes), asioErr);
      }
      if (asioErr) {
        std::cout << "Could not read response from peer: " << asioErr.message();
        return "";
      }
      return response;
    }
  };
  asio::io_context& GetAsioContext() { return m_asioContext; }
  
  std::string GetBody(const std::string& response) {
    int bodyStart = 4;
    while (
      (response[bodyStart-1] != '\n' ||
      response[bodyStart-2] != '\r' ||
      response[bodyStart-3] != '\n' ||
      response[bodyStart-4] != '\r') &&
      bodyStart < response.size()) bodyStart++;
    if (bodyStart >= response.size()) {
      std::cout << "GetBody(): could not parse response.\n";
      return "GET_BODY_ERROR";
    }
    return std::string(response.begin() + bodyStart, response.end());
  }
  Endpoint GetEndpoint(std::string url) {
    Endpoint endpoint;
    std::string port;
    ParseUrl(url, endpoint.domain, port, endpoint.path);
  
    asio::ip::tcp::resolver resolver(m_ioService);
    asio::ip::tcp::resolver::query query(endpoint.domain, port);
    asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, m_asioErr);
    if (m_asioErr) std::cout << "Could not resolve: " << m_asioErr.message();

    endpoint.asioEndpoint = *iter;
    return endpoint;
  }
  static Endpoint GetEndpoint(std::string ip, int port) {
    Endpoint endpoint;
    endpoint.asioEndpoint = asio::ip::tcp::endpoint(asio::ip::make_address(ip), port);
    return endpoint;
  }
  bool MakeGetReq(Endpoint endpoint, const std::string args, std::string& response) {
    asio::ip::tcp::socket socket(m_asioContext);
    socket.connect(endpoint.asioEndpoint, m_asioErr);
    if (m_asioErr) {
      std::cout << "Connection failed: " << m_asioErr.message();
      return false;
    }
    std::string request =
		  "GET " + endpoint.path + args +  " HTTP/1.1\r\n"
			"Host: " + endpoint.domain + "\r\n"
		  "Connection: close\r\n\r\n";
    // Send request
    socket.write_some(asio::buffer(request.data(), request.size()), m_asioErr);
    // Wait for the response
    socket.wait(socket.wait_read);
    int bytes = socket.available();
    if (bytes > 0) {
      response.resize(bytes);
			socket.read_some(asio::buffer(response.data(), bytes), m_asioErr);
    }
    if (m_asioErr) {
      std::cout << "Could not read response: " << m_asioErr.message();
      return false;
    }
    return true;
  }

private:
  asio::error_code m_asioErr;
  asio::io_context m_asioContext;
  asio::io_service m_ioService;

  void ParseUrl(const std::string& url, std::string &newUrl, std::string &port, std::string& path) {
    newUrl.clear();
    port.clear();
    char *domainStart = (char*)url.c_str() + 2;
    while (*(domainStart - 1) != '/' && *(domainStart - 2) != '/')
      domainStart++;
    while (*(++domainStart) != ':')
      newUrl += *domainStart;
    domainStart++; // Skip ':'
    while (*(domainStart) != '/')
      port += *(domainStart++);
    path = std::string(domainStart);
  }
};
