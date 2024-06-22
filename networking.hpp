#pragma once
#include "include/asio.hpp"
#include "include/asio/ts/buffer.hpp"
#include "include/asio/ts/internet.hpp"

class Networking {
public:
  Networking() : m_socket(asio::ip::tcp::socket(m_asioContext)) {}
  struct Endpoint {
    asio::ip::tcp::endpoint asioEndpoint;
    std::string domain, path, port;
  };
  Endpoint GetEndpoint(std::string url) {
    Endpoint endpoint;
    ParseUrl(url, endpoint.domain, endpoint.port, endpoint.path);
  
    asio::ip::tcp::resolver resolver(m_ioService);
    asio::ip::tcp::resolver::query query(endpoint.domain, endpoint.port);
    asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, m_asioErr);
    if (m_asioErr) std::cout << "Could not resolve: " << m_asioErr.message();

    endpoint.asioEndpoint = *iter;
    return endpoint;
  }
  bool MakeGetReq(Endpoint endpoint, const std::string args, std::vector<char>& response) {
    m_socket.connect(endpoint.asioEndpoint, m_asioErr);
    if (m_asioErr) {
      std::cout << "Connection failed: " << m_asioErr.message();
      return false;
    }
    std::string request =
		  "GET " + endpoint.path + args + " HTTP/1.1\r\n"
			"Host: " + endpoint.domain + "\r\n"
		  "Connection: close\r\n\r\n";
    // Send request
    m_socket.write_some(asio::buffer(request.data(), request.size()), m_asioErr);
    // Wait for the response
    m_socket.wait(m_socket.wait_read);
    int bytes = m_socket.available();
    if (bytes > 0) {
      response.resize(bytes);
			m_socket.read_some(asio::buffer(response.data(), bytes), m_asioErr);
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
  asio::ip::tcp::socket m_socket;

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
