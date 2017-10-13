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

    process_message( line );

    _socket_client.close();
  }
  else
  {
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
	  cout << "No message to be sent" << endl;
    return false;
  }
}

bool WebServer::score_compare(std::pair<string,int> pair1,
								std::pair<string,int> pair2){
	return (pair1.second < pair2.second);
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

      if( !res ){
    	  // Send response to client
    	  string res_str = res?"OK":"KO";
    	  do_send(res_str + "\n");
    	  return;
      }

      if( score.find("+") == std::string::npos and score.find("-") == std::string::npos){
          cout << "INFO: Introduced score user:" << user << " score:" << score << endl;
          int score_int = std::stoi(score);
          _user_score_map.push_back(std::make_pair(user, score_int));

          // Sort elements with new values
          std::sort (_user_score_map.begin(), _user_score_map.end(), score_compare);
      }else{
    	  string operation = score.substr(0,1);
    	  int value = std::stoi(score.substr(1,score.size()-1));

		  auto it = std::find_if( _user_score_map.begin(), _user_score_map.end(),
		      [&user](const std::pair<std::string, int>& element){ return element.first == user;} );
		  if (it != _user_score_map.end()){
	    	  if( operation.find("+") != std::string::npos ){
	    		  it->second += value;
	    	  }else if( operation.find("-") != std::string::npos ){
	    		  it->second -= value;
	    	  }else{
	    		  res = false;
	    	  }
		  }else{
			  res = false;
		  }

      }

	  // Send response to client
	  string res_str = res?"OK":"KO";
	  do_send(res_str + "\n");

  }
  else if (input.find("List") == 0)
  {
	  ptree pt;
	  ptree score_list_pt;
	  for( auto item : _user_score_map ){
		  ptree child;
		  child.put("user", item.first);
		  score_list_pt.push_back(std::make_pair("", child));
		  child.put("score", std::to_string(item.second));
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
	  int elements = std::stoi (input.substr(3, input.size()-3) );
	  cout << "INFO: Getting Top " << elements << endl;
	  ptree pt;
	  ptree score_list_pt;

	  std::vector<std::pair<string,int>>::reverse_iterator rit = _user_score_map.rbegin();
	  for (; rit!= _user_score_map.rend(); ++rit){
		  ptree child;
		  child.put("user", rit->first);
		  score_list_pt.push_back(std::make_pair("", child));
		  child.put("score", std::to_string(rit->second));
		  score_list_pt.push_back(std::make_pair("", child));
		  elements --;
		  if( elements == 0) break;

	  }
	  pt.add_child("score_list", score_list_pt);

	  std::ostringstream buf;
      write_json (buf, pt, false);
      std::string response = buf.str(); // {"foo":"bar"}
      do_send(response + "\n");
  }
  else if (input.find("At") == 0)
  {
	  size_t delimiter_index = input.find("/");
	  int n_index = std::stoi(input.substr(2, delimiter_index - 2));
	  int elements = std::stoi(input.substr(delimiter_index+1, input.size() - delimiter_index ));

	  int min_index = n_index - elements;
	  int max_index = n_index + elements;
	  if( min_index <  0) res = false;
	  if( max_index >  _user_score_map.size() - 1) res = false;

	  if( !res ){
		  // Send response to client
		  string res_str = res?"OK":"KO";
		  do_send(res_str + "\n");
	  }

	  ptree pt;
	  ptree score_list_pt;
	  for( int i = min_index; i < max_index+1; ++i ){
		  ptree child;
		  child.put("user", _user_score_map[i].first);
		  score_list_pt.push_back(std::make_pair("", child));
		  child.put("score", std::to_string(_user_score_map[i].second));
		  score_list_pt.push_back(std::make_pair("", child));


	  }
	  pt.add_child("score_list", score_list_pt);


	  std::ostringstream buf;
      write_json (buf, pt, false);
      std::string response = buf.str(); // {"foo":"bar"}
      do_send(response + "\n");


  }
  else
  {
	  do_send("command not recognized\n");
  }




}
