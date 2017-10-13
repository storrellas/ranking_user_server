#include <iostream>

#include <src/WebServer.h>

using namespace std;

int main()
{
  // Initilialise AsteriskMockup
//  _asterisk_mockup = new AsteriskMockup(&_weh);
//  _asterisk_mockup->set_event_handler(this);
//  if( ENABLE_ASTERISK_MOCKUP ){
//	_asterisk_mockup->init();
//  }

	WebServer web_server;
	web_server.init();
	web_server.accept_worker();

    cout << "Hello World!" << endl;
    return 0;
}

