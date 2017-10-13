#include <iostream>

#include <src/AsteriskMockup.h>

using namespace std;

int main()
{
  // Initilialise AsteriskMockup
//  _asterisk_mockup = new AsteriskMockup(&_weh);
//  _asterisk_mockup->set_event_handler(this);
//  if( ENABLE_ASTERISK_MOCKUP ){
//	_asterisk_mockup->init();
//  }

	AsteriskMockup asterisk_mockup;
	asterisk_mockup.init();
	asterisk_mockup.accept_worker();

    cout << "Hello World!" << endl;
    return 0;
}

