#include "rbcserial.hpp"


extern "C" void task(RbcSerial & r, char ** argv, std::ostream & o) {
	o << "DIAGTOOL TASK" << std::endl;

	// Commands

	unsigned char commandDirectMode[] = {
		COMMON_HEADER
		0x10, 0x01, 0x00, 0x00,
		0x00, 0x01, 0x01, 0x01
	};

	unsigned char commandFirstCheck[] = {
		COMMON_HEADER
		0x10, 0x01, 0x00, 0x00,
		0x00, 0x01, 0x01, 0x01,
	};

	unsigned char commandServoCheck[] = {
		0xFF, 0x00, 0x00, 0x00
	};

	unsigned char commandClosure[] = {
		0xFF, 0xE0, 0xFB, 0x01,
		0x00, 0x1A
	};

	// Responses

	o << "Robobuilder Direct Mode on" << std::endl;
	r.async_write(commandDirectMode);
	sleep(2);

	
	r.async_write(commandFirstCheck);
	r.flush();
	sleep(1);
	
	unsigned char i = 0xA0, j = 0x20;
	int ID = 0;

	for(; i <= 0xAF && j <= 0x2F; i ++, j ++) {
		o << "Servo ID " << std::setbase(10) << ID ++ << std::endl;
		commandServoCheck[1] = i;
		commandServoCheck[3] = j;
		unsigned char sresponse[2];
		r.async_write(commandServoCheck, 4);	
		r.read(sresponse, 2);

		r.print_bytes(sresponse, 2, o);
		o << "\t";

		if(* (sresponse + 1) == 0x00) {
			o << "Did not respond" << std::endl;
			break;
		}

		o << "Checked" << std::endl;
	
		r.flush();
		sleep(1);
	}

	r.async_write(commandClosure, 6);
	r.flush();
	sleep(2);
	o << "Robobuilder Direct Mode off" << std::endl;
}
