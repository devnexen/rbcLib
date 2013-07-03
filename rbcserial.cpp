#include "rbcserial.hpp"

/**
 * Open serial port
 * NO PARITY
 * 8 BITS
 * 1 STOP BIT
 * NO FLOW CONTROL
 */



void signal_handler(const boost::system::error_code & e, int sig) {
        if(!e) {
                std::cerr << "Signal received " << sig << std::endl;
        }
}

void RbcSerial::open() {
	boost::system::error_code ec;

	serial = boost::shared_ptr<boost::asio::serial_port>(new boost::asio::serial_port(io));

	try {
		std::cerr << "Serial port opening" << std::endl;
		serial->open(this->tty, ec);
	
		if(serial->is_open()) {
			serial->set_option(boost::asio::serial_port::baud_rate(brate));
			serial->set_option(opt_parity);
			serial->set_option(opt_csize);
			serial->set_option(opt_flow);
			serial->set_option(opt_stop);
				
	                boost::asio::signal_set signals(io, SIGTERM, SIGINT);
	                signals.async_wait(signal_handler);
	                	
	                boost::thread t(boost::bind(& boost::asio::io_service::run, & io));
		}
	} catch(std::exception & e) {
		std::cerr << "open Exception : " << e.what() << std::endl;
		close();
	}
}

RbcSerial::~RbcSerial() {
	close();
}

void RbcSerial::close() {
	if(serial && serial.get() && serial->is_open()) {
		std::cerr << "Serial port close" << std::endl;
		serial->cancel();
		serial->close();
		serial.reset();
	}

	io.stop();
	io.reset();
}

void RbcSerial::reset() {
	serial.reset();
}

void RbcSerial::flush() {
	if(serial.get() != NULL && serial->is_open()) {
		bool isflushed =! ::tcflush(serial->native(), TCIFLUSH);

		if(!isflushed) {
			boost::system::error_code ec;
			ec = boost::system::error_code(errno, boost::asio::error::get_system_category());
			std::cerr << "Flush error " << ec << std::endl;
		}
	}
}

/**
 * Synchrone writing
 */

void RbcSerial::write(const unsigned char * data, size_t size) {
	if(serial.get() != NULL && serial->is_open()) {
		try {
			serial->write_some(boost::asio::buffer(data, size));
		} catch(std::exception & e) {
			std::cerr << "write Exception " << e.what() << std::endl;
		}
	}
}

/**
 * Asynchrone writing
 */

void RbcSerial::async_write(const unsigned char * data, size_t size) {
	if(serial.get() != NULL && serial->is_open()) {
		size = size > RBC_BUFFER_SIZE ? RBC_BUFFER_SIZE : size;

		try {
			serial->async_write_some(boost::asio::buffer(data, size),
					boost::bind(& RbcSerial::async_write_handler, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));	
		} catch(std::exception & e) {
			std::cerr << "async write Exception " << e.what() << std::endl;
		}
	}
}



void RbcSerial::async_write_handler(const boost::system::error_code & e, size_t b) {
	if(e) {
		return;
	}

	serial->cancel();
}

/**
 * Synchrone reading
 */

void RbcSerial::read(unsigned char * data, size_t size) {
	if(serial.get() != NULL && serial->is_open()) {
		size = size > RBC_BUFFER_SIZE ? RBC_BUFFER_SIZE : size;
		boost::system::error_code ec;

		try {
			serial->read_some(boost::asio::buffer(data, size), ec);
		} catch(std::exception & e) {
			std::cerr << e.what() << std::endl;
		}
	}
}

/**
 * Asynchrone reading
 */

void RbcSerial::async_read(unsigned char * data, size_t size) {
	if(serial.get() != NULL && serial->is_open()) {
		size = size > RBC_BUFFER_SIZE ? RBC_BUFFER_SIZE : size;

		try {
			serial->async_read_some(boost::asio::buffer(data, size),
					boost::bind(& RbcSerial::async_read_handler, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));	
		} catch(std::exception & e) {
			std::cerr << e.what() << std::endl;
		}
	}
}

void RbcSerial::async_read_handler(const boost::system::error_code & e, std::size_t b) {
	if(e || !b) {
		return;
	}
}

/**
 * Handy method for writing array of bytes
 */

void RbcSerial::print_bytes(unsigned char * data, size_t size, std::ostream & o) {
	unsigned int i = 0;

        o << std::showbase << std::internal << std::setfill('0');

	while(i < size) {
		o << std::hex << (unsigned int) * (data + i) << " ";
		++ i;
	}

	o << std::dec << std::endl;
}

