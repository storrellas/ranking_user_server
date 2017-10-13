/*
 * TwilioMockup.cpp
 */

#include "WebServer.h"



namespace {

} // namespace

const int WebServer::LOCAL_PORT_INT;

WebServer::WebServer() :
  _io_service(),
  _socket(_io_service),
  _socket_client(_io_service),
  _acceptor(_io_service)
{
}

WebServer::~WebServer(){
}

void WebServer::init(const int& port){

  cout << "INFO: Initialising Asterisk Mockup" << endl;

  // Initialise acceptor
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  _acceptor.open(endpoint.protocol());
  _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  _acceptor.bind(endpoint);
  _acceptor.listen();
//
//  // Initialise worker
//  _th_accept_worker =
//      new boost::thread(boost::bind(&AsteriskMockup::accept_worker, this));


}

void WebServer::stop(){
  _socket.close();
  _socket_client.close();
  _io_service.stop();
}

void WebServer::reset(){
  _io_service.reset();
}

// -----------------------
// Commmunication functions
// -----------------------

void WebServer::accept_worker(){

  cout << "INFO: Started accept_worker" << endl;
  try
  {
    do_accept();
    _io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << endl;
  }
  std::cout << "Terminated accept_worker " << endl;
  //LLOG_INFO(LogItem(undefined, "Terminated accept_worker"));
}

void WebServer::do_accept(){

  _acceptor.async_accept(_socket,
      [this](boost::system::error_code ec)
      {
        if (!ec)
        {
          _socket_client = std::move(_socket);
          boost::asio::async_read_until(_socket_client, streambuf_, "\n",
              boost::bind(&WebServer::do_read, this, boost::asio::placeholders::error));

        }

        do_accept();
      });
}

void WebServer::do_read(const boost::system::error_code& err){

  if (!err)
  {

//    // NOTE: Adds \n at the end
//    std::istream response_stream(&streambuf_);
//    std::string line;
//    response_stream >> line;

    // Grab received
    std::istream is(&streambuf_);
    std::string line;
    std::getline(is, line);
//    ami_message_ += line + "\n";
//
//    // If we received '\n' means that message was finished
//    if( line.empty() ){
//      process_message(ami_message_);
//      ami_message_.clear();
//    }

    process_message( line );

    _socket_client.close();
//    boost::asio::async_read_until(_socket_client, streambuf_, "\n",
//        boost::bind(&WebServer::do_read, this, boost::asio::placeholders::error));

  }
  else
  {
    //LLOG_WARNING(LogItem(undefined, "Error receiving: " + err.message()));
    cout << "Error receiving: " + err.message() << endl;
  }

}

bool WebServer::do_send(const string& data){
  if( !data.empty() ){

    boost::system::error_code ignored_error;
    boost::asio::write(_socket_client, boost::asio::buffer(data),
        boost::asio::transfer_all(), ignored_error);
    return true;
  }else{
    //LLOG_ERROR(LogItem(undefined, "No message to be sent"));
	  cout << "No message to be sent" << endl;
    return false;
  }
}

void WebServer::process_message(string input){

  std::lock_guard<std::mutex> guard(_operation_mutex);
  cout << "INFO: Processing message -> " << input << endl;
  bool res = true;

  if (input.find("{") == 0)
  {
      // Read json.
      ptree pt;
      std::istringstream is (input);
      read_json (is, pt);
      std::string user = pt.get<std::string> ("user");
      if( user.empty() ) res = false;
      std::string score = pt.get<std::string> ("score");
      if( user.empty() ) res = false;

      if( res ){
          cout << "INFO: Introduced score user:" << user << " score:" << score << endl;
          _user_score_map[user] = score;
      }

	  // Send response to client
	  string res_str = res?"OK":"KO";
	  do_send(res_str + "\n");

  }
  else if (input.find("list") == 0)
  {
	  ptree pt;
	  ptree score_list_pt;
	  for( auto item : _user_score_map ){
		  ptree child;
		  child.put(item.first, item.second);
		  score_list_pt.push_back(std::make_pair("", child));
	  }
	  pt.add_child("score_list", score_list_pt);

	  std::ostringstream buf;
      write_json (buf, pt, false);
      std::string response = buf.str(); // {"foo":"bar"}
      do_send(response + "\n");
  }
  else if (input.find("Top") == 0)
  {
	  string elements = input.substr(3, input.size()-3);
	  cout << "Getting Top of " << elements << endl;
  }
  else
  {
	  do_send("command not recognized\n");
  }




}
