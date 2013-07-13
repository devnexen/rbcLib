#include "rbcserial.hpp"
#include "plugin.hpp"

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

	unsigned char currentPos, maxPos;
	SERVO_CHECK_BEGIN
		currentPos = * (sresponse + 1);
		maxPos = currentPos + 0x20;

		while(++ currentPos < maxPos) {
			RBC_SERVO_MOVE(ID, 3, currentPos)

			r.async_write(commandServoPos, 4);
			r.flush();
		}
	
		RBC_SERVO_PASSIVE(ID)
		r.async_write(commandServoPassive, 4);
		r.flush();

		sleep(1);
	SERVO_CHECK_END
}
