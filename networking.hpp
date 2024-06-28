#pragma once
#include "include/asio.hpp"
#include "include/asio/ts/buffer.hpp"
#include "include/asio/ts/internet.hpp"
#include "utils.hpp"

class Networking {
public:
  struct Endpoint {
    asio::ip::tcp::endpoint asioEndpoint;
    std::string domain, path;
  };
  struct Peer {
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::socket socket;
    std::string response;
    std::string handshake;
    Peer(Networking& networking, std::string handshake, std::string ip, int port) :
      socket(networking.GetAsioContext()), handshake(handshake) {
      endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(ip), port);
    }
    void Connect() {
      socket.async_connect(endpoint, [&](asio::error_code err) {
        std::string peerIp = endpoint.address().to_string();
        if (err)
          std::cout << "Could not connect to peer: " << peerIp << "\n"; //<< ": " << err.message() << "\n";
        else {
          std::cout << "Connected to peer successfully: " << peerIp << "\n";
          SendHandshake();
        }
      });
    }
    bool SendHandshake() {
      if (!socket.is_open()) {
        std::cout << "Could not send data to peer: socket not open.\n";
        return false;
      }
      asio::error_code err;
      socket.async_write_some(
        asio::buffer(handshake.data(), handshake.size()),
        [&](asio::error_code err, std::size_t) {
          if (err) std::cout << "Could not send handshake.\n";
          else RecvHandshake();
        }
      );
      return true;
    }
    void RecvHandshake() {
      response.resize(68); // If the response is more than 68 bytes then it's probably garbage
      asio::async_read(
        socket, asio::buffer(response.data(), response.size()),
        [&](const asio::error_code err, std::size_t) {
          std::string receivedInfoHash(response.begin() + 28, response.begin() + 47);
          std::string infoHash(handshake.begin() + 28, handshake.begin() + 47);
          if (receivedInfoHash == infoHash)
            std::cout << "Info hash matches!\n";
          else
            std::cout << "Info hash doesn't match.\n";
        }
      );
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
