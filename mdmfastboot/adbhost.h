/*=============================================================================
DESC:
   Multi-Port support Version base on adb protocol.

CHANGE HISTORY:
when        who          what
----------  ---------    --------------------------------------------------------
2013-07-10  shenlong.xu  Init first version

=============================================================================*/
#pragma once  // replace
							//			#define __ADBHOST_H
              //      #define __ADBHOST_H
              //      ... ...
              //      #endif
#include "sysdeps.h"
#include "adb.h"
class adbhost
{
	public:
		adbhost(void);
		adbhost(usb_handle *usb, unsigned address);
		~adbhost(void);

		void process(void);

	private:
		//void handle_packet(apacket *p, atransport *t);
		void open_service(const char *destination);
		void connect(atransport *t);
		int adbhost::command() ;

		int read_packet(atransport *t, apacket** ppacket);
		int write_packet(apacket** ppacket);
		void send_packet(apacket *p, atransport *t);
		bool receive_packet(apacket *p);

		bool handle_connect_response(void);
		bool handle_open_response(void);
		bool handle_command_response (void);

 	  void notify_ready_to_remote(void);
		void close_remote(void);
		int enqueue_command(apacket *p);

	private:
		atransport t;
		unsigned id; //local id, address
		unsigned peer_id;
};
