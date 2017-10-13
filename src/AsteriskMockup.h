#pragma once

#include <regex>
#include <string>
#include <iostream>
#include <map>
 #include <mutex>

// Boost includes
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

const string ASTERISK_MOCKUP_VERSION ="0.1";

class AsteriskMockup
{
public:
  /**
   * @brief LOCAL_PORT local port configured
   */
  const static int LOCAL_PORT_INT = 8080;

private :
  std::mutex _operation_mutex;

  /**
   * @brief receive_worker
   */
  boost::thread* _th_accept_worker;

  /**
   * @brief _io_service Used to start service for IO operations
   */
  boost::asio::io_service _io_service;

  /**
   * @brief streambuf_ Holds received contents
   */
  boost::asio::streambuf streambuf_;

  /**
   * @brief socket_ Socket to accept requests
   */
  boost::asio::ip::tcp::socket _socket;

  /**
   * @brief socket_client_ Socket when connection received
   */
  boost::asio::ip::tcp::socket _socket_client;

  /**
   * @brief _acceptor Object to accept connections
   */
  boost::asio::ip::tcp::acceptor _acceptor;

  /**
   * @brief ami_message_ Stores AMI message
   */
  std::string ami_message_;


public:

  AsteriskMockup();
  virtual ~AsteriskMockup();


  /**
   * Initialises the Twilio controller
   */
  void init(const int& port = LOCAL_PORT_INT);

public:


  /**
   * @brief accept_worker thread aimed at receiving information from PBXServer
   */
  void accept_worker();

  /**
   * @brief do_accept Performs accept operations
   */
  void do_accept();

  /**
   * @brief do_read Performs read operation
   * @param err
   */
  void do_read(const boost::system::error_code& err);

  /**
   * @brief send_data Sends data through socket
   * @param data
   */
  bool do_send(const string& data);

  /**
   * @brief process_ami_message Processes AMI message
   * @param msg
   */
  void process_message(string msg);


};

