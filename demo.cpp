#include "CPololuMaestro.h"

#include <iostream>
#include <string>
using namespace std;

int main()
{
	try
	{
		CPololuMaestro m("/dev/ttyACM0");
		
		m.setTarget(0, 992*4);
		sleep(1);
		m.setTarget(0, 2500*4);
		sleep(1);
	
		m.setTarget(0, 0);
	}
	catch (string error)
	{
		cout << "Error:" << error << endl;
	}
	catch (...)
	{
		throw;
	}
	return 0;
}
