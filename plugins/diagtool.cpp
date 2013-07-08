#include "rbcserial.hpp"
#include "plugin.hpp"
#include <termios.h>

static bool warned = false;
void plugServosChecking(RbcSerial &, std::ostream &);
void servosMovesChecking(RbcSerial &, std::ostream &);

extern "C" int menu(void) {
	MENU_VARS

	printf("1/ Plug servos checking\n");
	printf("2/ Servos moves checking\n");
	
   	GET_MENU_CHOICE	
}

extern "C" void task(RbcSerial & r, int c, int argc, char ** argv, std::ostream & o) {
	void (* checking)(RbcSerial &, std::ostream &);

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

	checking(r, o);
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

void plugServosChecking(RbcSerial & r, std::ostream & o) {
	o << "PLUG SERVOS CHECKING" << std::endl;
	// Commands

	unsigned char commandFirstCheck[] = {
		COMMON_HEADER
		0x10, 0x01, 0x00, 0x00,
		0x00, 0x01, 0x01, 0x01,
	};

	r.async_write(commandFirstCheck);
	r.flush();
	sleep(1);
	
	SERVO_CHECK_BEGIN
	SERVO_CHECK_END
}

void servosMovesChecking(RbcSerial & r, std::ostream & o) {
	o << "SERVOS MOVE CHECKING" << std::endl;
	if(!warned) {
		o << "Warning, Robo may fall, should not stand :-)" << std::endl;
		warned = true;
		sleep(2);
	}

	struct termios old, cur;
	tcgetattr(fileno(stdin), & old);
	cur = old;
	cur.c_lflag &= ~ICANON;
	cur.c_lflag &= ~ECHO;

	tcsetattr(fileno(stdin), TCSANOW, & cur);

	unsigned char currentPos, maxPos, minPos;
	SERVO_CHECK_BEGIN
		maxPos = currentPos = * (sresponse + 1);

		while((maxPos % 0x20) != 0) {
			++ maxPos;
		}

		minPos = maxPos - 0x06;
		r.print_bytes(& currentPos, 1, o);
		r.print_bytes(& maxPos, 1, o);
		r.print_bytes(& minPos, 1, o);

		while(1) {
			fd_set set;
			struct timeval tv;

			tv.tv_sec = 2;
			tv.tv_usec = 0;

			ushort v = 0;
			while(currentPos < maxPos) {
				++ currentPos;
				unsigned char posB = (currentPos < 0x80 ? currentPos - 0x60 : 0x60);
				RBC_MOVE_CHECK(ID, v, currentPos, posB)

				unsigned char sresponse[34];

				r.async_write(commandMove, 70);
				r.read(sresponse, 34);				

				++ v;
			}

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

			v = 0;
			while(currentPos > minPos) {
				-- currentPos;
				unsigned char posB = currentPos - 0x60;
				RBC_MOVE_CHECK(ID, v, currentPos, posB)

				unsigned char sresponse[34];

				r.async_write(commandMove, 70);
				r.read(sresponse, 34);				

				++ v;
			}
		}
	SERVO_CHECK_END

	tcsetattr(fileno(stdin), TCSANOW, & old);
}
