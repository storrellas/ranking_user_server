#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

// System includes
#include <iostream>
#include <map>

// Boost includes
#include <boost/array.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

// Project includes
#include <src/WebServer.h>


using namespace std;
using namespace boost;
using boost::asio::ip::tcp;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;


const string SERVER_IP = "localhost";
const string SERVER_PORT = "8080";

// Global variables
boost::thread* _th_accept_worker;
WebServer* web_server;


void _do_send_and_receive(const string& request, string& response){

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

string _generate_json(const ptree pt){
    std::ostringstream buf;
    write_json (buf, pt, false);
    return buf.str();
}


string _introduce_score(const string user, const string score){
    ptree pt;
    pt.put("user", user);
    pt.put("score", score);

    std::string json = _generate_json(pt);

    string response;
    _do_send_and_receive(json, response);
    cout << "INFO: Read from server -> " << response << endl;
    return response;

}


//____________________________________________________________________________//

struct WebServerManagement {
	WebServerManagement()   {
	  cout << "INFO: Starting WebServer" << endl;
	  web_server = new WebServer();
	  web_server->init();
	  _th_accept_worker =
		  new boost::thread(boost::bind(&WebServer::accept_worker, web_server));
    }
    ~WebServerManagement()  {
	  cout << "INFO: Terminating WebServer" << endl;
	  web_server->_io_service.stop();
	  cout << "INFO: Terminating _th_accept_worker" << endl;
	  _th_accept_worker->join();
	  web_server->_io_service.reset();
	  delete web_server;
	  delete _th_accept_worker;
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE( WebServerManagement );

BOOST_AUTO_TEST_CASE( score_compare )
{
    cout << "+++++++++++++++++++++++++++" << endl;
    cout << "INFO: score_compare" << endl;
    cout << "+++++++++++++++++++++++++++" << endl;

	std::pair<string, int> pair1("123", 456);
	std::pair<string, int> pair2("345", 879);

	BOOST_CHECK( WebServer::score_compare(pair1, pair2) ); // Means pair1 < pair2
}

BOOST_AUTO_TEST_CASE( score_introduced )
{
    cout << "+++++++++++++++++++++++++++" << endl;
    cout << "INFO: score_introduced" << endl;
    cout << "+++++++++++++++++++++++++++" << endl;

	// 1. Introduce scores
	_introduce_score("123", "789");
	_introduce_score("456", "345");

	// 2. List
	string response;
    _do_send_and_receive("List\n", response);
    cout << "INFO: Read from server -> " << response << endl;

    // Analyse reponse
    ptree pt;
    std::istringstream is (response);
    read_json (is, pt);

    ptree score_list_pt = pt.get_child("score_list");
    BOOST_CHECK( score_list_pt.size() == 2);

    // Clear WebServer cache
    web_server->clear();
}


BOOST_AUTO_TEST_CASE( score_operator )
{
    cout << "+++++++++++++++++++++++++++" << endl;
    cout << "INFO: score_operator" << endl;
    cout << "+++++++++++++++++++++++++++" << endl;

	// 1. Introduce scores
	_introduce_score("123", "700");
	_introduce_score("123", "+20");
	_introduce_score("123", "-40");

	// 2. List
	string response;
    _do_send_and_receive("List\n", response);
    cout << "INFO: Read from server -> " << response << endl;

    // Analyse reponse
    ptree pt;
    std::istringstream is (response);
    read_json (is, pt);

    ptree score_list_pt	 = pt.get_child("score_list");
    BOOST_CHECK( score_list_pt.size() == 1);

    bool res = false;
    BOOST_FOREACH(ptree::value_type& child, score_list_pt){
    	if( child.second.get<string>("score") == "680" ) res = true;
    }
    BOOST_CHECK( res );

    // Clear WebServer cache
    web_server->clear();
}

BOOST_AUTO_TEST_CASE( top )
{
    cout << "+++++++++++++++++++++++++++" << endl;
    cout << "INFO: top" << endl;
    cout << "+++++++++++++++++++++++++++" << endl;

	// 1. Introduce scores
	_introduce_score("123", "789");
	_introduce_score("456", "345");
	_introduce_score("456", "123");
	_introduce_score("456", "002");

	// 2. List
	string response;
    _do_send_and_receive("List\n", response);
    cout << "INFO: Read from server -> " << response << endl;

	// 3. Top
    _do_send_and_receive("Top2\n", response);
    cout << "INFO: Read from server -> " << response << endl;

    // Analyse reponse
    ptree pt;
    std::istringstream is (response);
    read_json (is, pt);
    ptree score_list_pt	 = pt.get_child("score_list");
    BOOST_CHECK( score_list_pt.size() == 2 );

    // Clear WebServer cache
    web_server->clear();
}

BOOST_AUTO_TEST_CASE( at )
{
    cout << "+++++++++++++++++++++++++++" << endl;
    cout << "INFO: at" << endl;
    cout << "+++++++++++++++++++++++++++" << endl;


	// 1. Introduce scores
	_introduce_score("123", "789");
	_introduce_score("456", "345");
	_introduce_score("456", "123");
	_introduce_score("325", "002");
	_introduce_score("462", "472");
	_introduce_score("982", "458");
	_introduce_score("943", "231");

	// 2. List
	string response;
    _do_send_and_receive("List\n", response);
    cout << "INFO: Read from server -> " << response << endl;

	// 3. At
    _do_send_and_receive("At3/2\n", response);
    cout << "INFO: Read from server -> " << response << endl;

    // Analyse reponse
    ptree pt;
    std::istringstream is (response);
    read_json (is, pt);
    ptree score_list_pt	 = pt.get_child("score_list");
    BOOST_CHECK( score_list_pt.size() == 5 );

}
