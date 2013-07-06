#include "rbcserial.hpp"
#include "plugin.hpp"
#include <termios.h>

static bool warned = false;
static unsigned char servosPos[16] = { 0x00 };
void plugServosChecking(RbcSerial &, unsigned char [16], std::ostream &);
void servosMovesChecking(RbcSerial &, unsigned char [16], std::ostream &);

inline bool servosPosArrayCheck(unsigned char servosPos[16]) {
	ushort i = 0;
	while(i < 16) {
		if(* (servosPos) + i == 0x00) {
			return false;
		}

		++ i;
	}

	return true;
}

extern "C" int menu(void) {
	MENU_VARS

	printf("1/ Plug servos checking\n");
	printf("2/ Servos moves checking\n");
	
   	GET_MENU_CHOICE	
}

extern "C" void task(RbcSerial & r, int c, int argc, char ** argv, std::ostream & o) {
	void (* checking)(RbcSerial &, unsigned char [16], std::ostream &);

	switch(c) {
		case 1:
			checking = plugServosChecking;
			break;
		case 2:
			checking = servosMovesChecking;
			break;
		default:
			std::cerr << "Bad choice" << std::endl;
			return;
	}

	checking(r, servosPos, o);
}

extern "C" void start(RbcSerial & r, std::ostream & o) {
	unsigned char commandDirectMode[] = {
		COMMON_HEADER
		0x10, 0x01, 0x00, 0x00,
		0x00, 0x01, 0x01, 0x01
	};

	o << "Robobuilder Direct Mode on" << std::endl;
	r.async_write(commandDirectMode);
	sleep(2);
}

extern "C" void stop(RbcSerial & r, std::ostream & o) {
	unsigned char commandClosure[] = {
		0xFF, 0xE0, 0xFB, 0x01,
		0x00, 0x1A
	};

	r.async_write(commandClosure);
	r.flush();
	sleep(2);
	o << "Robobuilder Direct Mode OFF" << std::endl;	
}

void plugServosChecking(RbcSerial & r, unsigned char servosPos[16], std::ostream & o) {
	o << "PLUG SERVOS CHECKING" << std::endl;
	memset(servosPos, 0x00, 16);
	// Commands

	unsigned char commandFirstCheck[] = {
		COMMON_HEADER
		0x10, 0x01, 0x00, 0x00,
		0x00, 0x01, 0x01, 0x01,
	};

	unsigned char commandServoCheck[] = {
		0xFF, 0x00, 0x00, 0x00
	};

	
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
		} else {
			* servosPos = * (sresponse + 1);
			servosPos ++;
		}

		o << "Checked" << std::endl;
	
		r.flush();
		sleep(1);
	}

}

void servosMovesChecking(RbcSerial & r, unsigned char servosPos[16], std::ostream & o) {
	o << "SERVOS MOVE CHECKING" << std::endl;
	struct termios old, cur;
	tcgetattr(fileno(stdin), & old);
	cur = old;
	cur.c_lflag &= ~ICANON;
	cur.c_lflag &= ~ECHO;
	
	tcsetattr(fileno(stdin), TCSANOW, & cur);

	if(servosPosArrayCheck(servosPos)) {
		if(!warned) {
			o << "Warning, Robo may fall, should not stand :-)" << std::endl;
			warned = true;
			sleep(2);
		}

		unsigned char currentPos;

		currentPos = * servosPos;

		while(1) {
			fd_set set;
			struct timeval tv;

			tv.tv_sec = 10;
			tv.tv_usec = 0;

			FD_ZERO(& set);
			FD_SET(fileno(stdin), & set);

			int res = select(fileno(stdin) + 1, & set, 0, 0, & tv);

			if(res) {
				char c;
				read(fileno(stdin), & c, 1);
				
				if(c == ' ') {
					break;
				}
			}

			ushort i = 0;
			while(currentPos < 0x80) {
				++ currentPos;
				unsigned char posB = (currentPos < 0x80 ? currentPos - 0x60 : 0x60);
				unsigned char commandMove[70] = {
					RBC_MOVE_CHECK_0(i, currentPos, posB)
				};

				unsigned char sresponse[34];

				r.async_write(commandMove, 70);
				r.print_bytes(commandMove, 70, o);
				r.read(sresponse, 34);				

				++ i;
			}

			i = 0;
			while(currentPos > 0x7A) {
				-- currentPos;
				unsigned char posB = currentPos - 0x60;
				unsigned char commandMove[70] = {
					RBC_MOVE_CHECK_0(i, currentPos, posB)
				};

				unsigned char sresponse[34];

				r.async_write(commandMove, 70);
				r.print_bytes(commandMove, 70, o);
				r.read(sresponse, 34);				

				++ i;
			}
		}

		tcsetattr(fileno(stdin), TCSANOW, & old);
	} else {
		o << "Servos not well checked !" << std::endl;	
		return;
	}	
}
