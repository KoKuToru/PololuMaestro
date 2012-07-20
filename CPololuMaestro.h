/*
	Just a write down of 
	http://www.pololu.com/docs/0J40/5.e
	as C++ class
	
	Do what ever you want with this source.
	
	NOTE: The Maestro's serial mode must be set to "USB Dual Port" !
	NOTE: To get this working in ArchLinux
	      you must add yourself to the "uucp" group !
*/

#ifndef CPOLOLUMAESTRO
#define CPOLOLUMAESTRO

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include <string>
#include <string.h>
#include <errno.h>

/*
	WARNING NOT THREAD SAVE !
	+ USES strerror, AGAIN NOT THREADSAVE !
*/
class CPololuMaestro
{
	private:
		int fd;
		bool error_check;
		
	public:
		CPololuMaestro(std::string device): error_check(false) /* probably /dev/ttyACM0 */
		{
			fd = open(device.c_str(), O_RDWR | O_NOCTTY);
			if (fd == -1)
			{
				throw device + " open " + std::string(strerror(errno));
			}
			
			struct termios options;
			if (tcgetattr(fd, &options) == -1)
			{
				throw device + " tcgetattr " + std::string(strerror(errno));
			}
			
			options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			options.c_oflag &= ~(ONLCR | OCRNL);
						
			if (tcsetattr(fd, TCSANOW, &options) == -1) //<- doesn't work ?
			{
				throw device + " tcsetattr " + std::string(strerror(errno));
			}
		}
		
		~CPololuMaestro()
		{
			if (fd != 1)
			{
				close(fd);
				fd = -1;
			}
		}
		
		void setErrorCheck(bool error_check)
		{
			this->error_check = error_check;
			//this one is not good.. because you wont know the error codes.. but it will check for errors after each sent command.
		}
		
		void setTarget(int channel, int target, int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0x84, channel, target & 0x7F, (target >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setTarget" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x04, channel, target & 0x7F, (target >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setTarget" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("SetTarget error_check !");
				}
			}
		}
		
		void setMultipleTargets(const std::vector< std::pair<int, int> >& channel_targets, bool maestro_12pp=false, int device=-1)
		{
			if (!maestro_12pp)
			{
				//only works on meastor with more than (or equal) 12 channels
				for(int i = 0; i < channel_targets.size(); ++i)
				{
					setTarget(channel_targets[i].first, channel_targets[i].second, device);
				}
			}
			else
			{
				unsigned char data[24*3+4] = {0};
				unsigned char* pdata = data;
				if (device == -1)
				{
					unsigned char* pdata = data;
					*(pdata++) = 0x9F;
				}
				else
				{
					unsigned char* pdata = data;
					*(pdata++) = 0xAA;
					*(pdata++) = device;
					*(pdata++) = 0x1F;
				}
				*(pdata++) = channel_targets.size();					
				for(int i = 0; i < channel_targets.size(); ++i)
				{
					*(pdata++) = channel_targets[i].first;
					*(pdata++) = channel_targets[i].second & 0x7F;
					*(pdata++) = (channel_targets[i].second >> 7) & 0x7F;
				}
				if (write(fd, data, pdata-data) != (pdata-data))
				{
					throw "setMultipleTargets" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("setMultipleTargets error_check !");
				}
			}
		}
		
		void setSpeed(int channel, int speed, int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0x87, channel, speed & 0x7F, (speed >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setSpeed" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x07, channel, speed & 0x7F, (speed >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setSpeed" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("setSpeed error_check !");
				}
			}
		}
		
		void setAcceleration(int channel, int acceleration, int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0x89, channel, acceleration & 0x7F, (acceleration >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setAcceleration" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x09, channel, acceleration & 0x7F, (acceleration >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setAcceleration" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("setAcceleration error_check !");
				}
			}
		}
		
		//only works on meastor with more than (or equal) 12 channels
		void setPWM(int time, int periode, int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0x8A, time & 0x7F, (time >> 7) & 0x7F, periode & 0x7F, (periode >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setPWM" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x0A, time & 0x7F, (time >> 7) & 0x7F, periode & 0x7F, (periode >> 7) & 0x7F};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "setPWM" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("setPWM error_check !");
				}
			}
		}
		
		int getPosition(int channel, int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0x90, channel};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "getPosition" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x10, channel};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "getPosition" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("getPosition error_check !");
				}
			}
			unsigned short position = 0;
			if (read(fd, &position, sizeof(position)) != sizeof(position))
			{
				throw "getPosition" + std::string(strerror(errno));
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("getPosition error_check !");
				}
			}
			return position;
		}
		
		//Better use getPosition.. because getMovingState() is buggy on my device, it only works for channel 0 ! [Maestro 6 channel]
		bool getMovingState(int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0x93};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "getMovingState" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x13};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "getMovingState" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("getMovingState error_check !");
				}
			}
			unsigned char state = 0;
			if (read(fd, &state, sizeof(state)) != sizeof(state))
			{
				throw "getMovingState" + std::string(strerror(errno));
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("getMovingState error_check !");
				}
			}
			return state == 1;
		}
		
		void goHome(int device=-1)
		{
			if (device == -1)
			{
				const unsigned char data[] = {0xA2};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "goHome" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x22};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "goHome" + std::string(strerror(errno));
				}
			}
			if (error_check)
			{
				if (getErrors(device) != 0)
				{
					throw std::string("goHome error_check !");
				}
			}
		}
		
		int getErrors(int device=-1) //http://www.pololu.com/docs/0J40/4.b
		{
			if (device == -1)
			{
				const unsigned char data[] = {0xA1};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "getErrors" + std::string(strerror(errno));
				}
			}
			else
			{
				const unsigned char data[] = {0xAA, device, 0x22};
				if (write(fd, data, sizeof(data)) != sizeof(data))
				{
					throw "getErrors" + std::string(strerror(errno));
				}
			}
			unsigned short error = 0;
			if (read(fd, &error, sizeof(error)) != sizeof(error))
			{
				throw "getErrors" + std::string(strerror(errno));
			}
			return error; //0-7 and 8-15 bits.. ?
		}
};

#endif
