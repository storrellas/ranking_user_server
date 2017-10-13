// System includes
#include <iostream>
#include <map>

// Boost includes
#include <boost/array.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// Project includes
#include <src/WebServer.h>


using namespace std;
using namespace boost;
using boost::asio::ip::tcp;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

namespace {

const string SERVER_IP = "localhost";
const string SERVER_PORT = "8080";

// Global variables
boost::thread* _th_accept_worker;
WebServer* web_server;

void init(){
  cout << "INFO: Starting WebServer" << endl;
  web_server = new WebServer();
  web_server->init();
  _th_accept_worker =
	  new boost::thread(boost::bind(&WebServer::accept_worker, web_server));
}

void destroy(){
  cout << "INFO: Terminating WebServer" << endl;
  web_server->_io_service.stop();
  cout << "INFO: Terminating _th_accept_worker" << endl;
  _th_accept_worker->join();
  web_server->_io_service.reset();
  delete web_server;
  delete _th_accept_worker;
}

void do_send_and_receive(const string& request, string& response){

	boost::system::error_code error;
    boost::asio::io_service io_service;

    // Determine the location of the server.
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(SERVER_IP, SERVER_PORT);
    tcp::endpoint remote_endpoint = *resolver.resolve(query);

    // Establish the control connection to the server.
    tcp::socket socket(io_service);
    socket.connect(remote_endpoint);

    // Send request
    socket.write_some(boost::asio::buffer(request.data(), request.size()), error);

    // Get response
    boost::asio::streambuf response_sb;
    boost::asio::read_until(socket, response_sb, "\n");
    // Grab received
    std::istream is(&response_sb);
    std::string line;
    std::getline(is, response);

    socket.close();

}

}



void _01_score_introduced(){

	// 1. Introduce score
    ptree pt;
    pt.put("user", "123");
    pt.put("score", "789");

    std::ostringstream buf;
    write_json (buf, pt, false);
    std::string json = buf.str(); // {"foo":"bar"}
    cout << json << endl;

    string response;
	do_send_and_receive(json, response);
    cout << "INFO: Read from server -> " << response << endl;

	// 2. List
	do_send_and_receive("list\n", response);
    cout << "INFO: Read from server -> " << response << endl;
}

int main()
{
  // Initialise WebServer
  init();

  _01_score_introduced();

  // Destroy WebServer
  destroy();
  return 0;
}

