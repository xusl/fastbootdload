/*=============================================================================
DESC:
   Multi-Port support Version base on adb protocol.

CHANGE HISTORY:
when        who          what
----------  ---------    --------------------------------------------------------
2013-07-10  shenlong.xu  Init first version

=============================================================================*/
#include "StdAfx.h"

#include "adbhost.h"
#define   TRACE_TAG  TRACE_TRANSPORT

#if ADB_TRACE
static void  dump_hex( const unsigned char*  ptr, size_t  len )
{
   int  nn, len2 = len;
   //char* buffer = (char*)malloc (len * sizeof(char) * 4);

   //if (buffer == NULL)
   //	return;

   //memset(buffer, 0, len * sizeof(char) * 4);

   //if (len2 > 16) len2 = 16;

   for (nn = 0; nn < len2; nn++)
      D("%02x", ptr[nn]);
   D("  \n");

   for (nn = 0; nn < len2; nn++) {
      int  c = ptr[nn];
      if (c < 32 || c > 127)
         c = '.';
      D("%c", c);
   }
   D("\n");
   fflush(stdout);
   //free(buffer);
}
#endif

adbhost::adbhost(usb_handle *usb, unsigned address):
   id(address)
{
   //t = (atransport *)calloc(1, sizeof(atransport));
   memset(&this->t, 0 ,sizeof(atransport));
   init_usb_transport(&this->t, usb,  CS_OFFLINE);

}

adbhost::~adbhost(void)
{
   kick_transport(&this->t);
   //    transport_unref(&this->t);
   if (this->t.product != NULL)
      free(this->t.product);
}

int adbhost::read_packet(atransport *t, apacket** ppacket)
{
   if(t->read_from_remote(*ppacket, t) == 0){
      D("from_remote: received remote packet, sending to transport %p\n",
        t);
   } else {
      D("from_remote: remote read failed for transport %p\n", *ppacket);
      return -1;
   }

#if ADB_TRACE
   if (ADB_TRACING)
   {
      unsigned  command = (*ppacket)->msg.command;
      int       len     = (*ppacket)->msg.data_length;
      char      cmd[5];
      int       n;

      for (n = 0; n < 4; n++) {
         int  b = (command >> (n*8)) & 255;
         if (b >= 32 && b < 127)
            cmd[n] = (char)b;
         else
            cmd[n] = '.';
      }
      cmd[4] = 0;

      D("read_packet:  [%08x %s] %08x %08x (%d) ",
        command, cmd, (*ppacket)->msg.arg0, (*ppacket)->msg.arg1, len);
      dump_hex((*ppacket)->data, len);
   }
#endif
   return 0;
}

int adbhost::write_packet(apacket** ppacket)
{
   char *p = (char*) ppacket;  /* we really write the packet address */
   int r, len = sizeof(ppacket);

#if ADB_TRACE
   if (ADB_TRACING)
   {
      unsigned  command = (*ppacket)->msg.command;
      int       len     = (*ppacket)->msg.data_length;
      char      cmd[5];
      int       n;

      for (n = 0; n < 4; n++) {
         int  b = (command >> (n*8)) & 255;
         if (b >= 32 && b < 127)
            cmd[n] = (char)b;
         else
            cmd[n] = '.';
      }
      cmd[4] = 0;

      D("write_packet:  [%08x %s] %08x %08x (%d) ",
        command, cmd, (*ppacket)->msg.arg0, (*ppacket)->msg.arg1, len);
      dump_hex((*ppacket)->data, len);
   }
#endif

   t.write_to_remote(*ppacket, &t);
   return 0;
}

bool adbhost::receive_packet(apacket *p)
{
   int i=0;
   for(; ;){
      if(read_packet(&t, &p)){
         D("failed to read packet from usb\n");
         if (i++ < 3)
            sleep(1);
         else
            break;
      } else {
         return true;
      }
   }
   return false;
}

void adbhost::send_packet(apacket *p, atransport *t)
{
   unsigned char *x;
   unsigned sum;
   unsigned count;

   p->msg.magic = p->msg.command ^ 0xffffffff;

   count = p->msg.data_length;
   x = (unsigned char *) p->data;
   sum = 0;
   while(count-- > 0){
      sum += *x++;
   }
   p->msg.data_check = sum;

   print_packet("send", p);

   if (t == NULL) {
      fatal_errno("Transport is null");
      D("Transport is null \n");
   }

   if(write_packet(&p)){
      fatal_errno("cannot enqueue packet on transport socket");
   }
}

void adbhost::connect(atransport *t)
{
   apacket *cp = get_apacket();
   cp->msg.command = A_CNXN;
   cp->msg.arg0 = A_VERSION;
   cp->msg.arg1 = MAX_PAYLOAD;
   snprintf((char*) cp->data, sizeof cp->data, "%s::", "host" );
   cp->msg.data_length = strlen((char*) cp->data) + 1;
   D("Calling connect \n");
   send_packet(cp, t);

   /* XXX why sleep here? */
   // allow the device some time to respond to the connect message
   adb_sleep_ms(1000);

   put_apacket(cp);
}


void adbhost::open_service(const char *destination)
{
   apacket *p = get_apacket();
   int len = strlen(destination) + 1;
   D("Connect_to_remote call \n");

   if(len > (MAX_PAYLOAD-1)) {
      fatal("destination oversized");
   }

   D("LS(%d): connect('%s')\n", id, destination);
   p->msg.command = A_OPEN;
   p->msg.arg0 = this->id;//s->id;
   p->msg.data_length = len;
   strcpy((char*) p->data, destination);
   send_packet(p, &this->t);
   put_apacket(p);
}

int adbhost::command() {
   return 0;
}

int adbhost::enqueue_command(apacket *p)
{
   D("Calling remote_socket_enqueue\n");
   p->msg.command = A_WRTE;
   p->msg.arg0 = this->peer_id;
   p->msg.arg1 = this->id;
   p->msg.data_length = p->len;
   send_packet(p, &this->t);
   return 1;
}

void adbhost::notify_ready_to_remote()
{
   apacket *p = get_apacket();
   p->msg.command = A_OKAY;
   p->msg.arg0 = this->peer_id;
   p->msg.arg1 = this->id;
   D("Calling remote_socket_ready\n");
   send_packet(p, &this->t);
   put_apacket(p);
}

void adbhost::close_remote(void)
{
   apacket *p = get_apacket();

   p->msg.command = A_CLSE;
   p->msg.arg1 = this->id;
   D("Calling remote_socket_close\n");
   send_packet(p, &this->t);
   D("RS(%d): closed\n", this->id);

   // remove_transport_disconnect( s->transport, &((aremotesocket*)s)->disconnect );
   // free(s);
   put_apacket(p);
}

bool adbhost::handle_command_response (void) {
   apacket *p = get_apacket();
   if (p == NULL)
      return false;

   for (;;) {
      memset(p, 0, sizeof(apacket));
      p->msg.data_length = MAX_PAYLOAD;
      if(read_packet(&t, &p)){
      } else {
         break;
      }

   }

   put_apacket(p);
   return true;
}

bool adbhost::handle_open_response (void) {
   apacket *p =  get_apacket();
   bool result = true;

   if (p == NULL)
      return false;

   receive_packet(p);

   if ( p->msg.command == A_CLSE) {
      close_remote();
      result = false;
   } else if (p->msg.command == A_OKAY) {
      /* READY(local-id, remote-id, "") */
      DEBUG("handle receive: local-id=%d, remote-id=%d",
            p->msg.arg0,p->msg.arg1 );
      if(p->msg.arg1 != id) {
         ERROR("p->msg.arg1 (%d) not euqal id (%d)", p->msg.arg1, id);
         result = false;
      } else {
         this->peer_id = p->msg.arg0;
      }
   }
   put_apacket(p);
   return result;
}

bool adbhost::handle_connect_response(void)
{
   char *type, *product, *end;
   apacket *p = get_apacket();
   char * banner;

   receive_packet(p);

   if (p == NULL || p->msg.command != A_CNXN)
      return false;

   banner = (char*) p->data;

   D("parse_banner: %s\n", banner);
   type = banner;
   product = strchr(type, ':');
   if(product) {
      *product++ = 0;
   } else {
      product = "";
   }

   /* remove trailing ':' */
   end = strchr(product, ':');
   if(end) *end = 0;

   /* save product name in device structure */
   if (t.product == NULL) {
      t.product = strdup(product);
   } else if (strcmp(product, t.product) != 0) {
      free(t.product);
      t.product = strdup(product);
   }

   t.connection_state = CS_HOST;

   if(!strcmp(type, "bootloader")){
      D("setting connection_state to CS_BOOTLOADER\n");
      t.connection_state = CS_BOOTLOADER;
   }
   if(!strcmp(type, "device")) {
      D("setting connection_state to CS_DEVICE\n");
      t.connection_state = CS_DEVICE;
   }
   if(!strcmp(type, "recovery")) {
      D("setting connection_state to CS_RECOVERY\n");
      t.connection_state = CS_RECOVERY;
   }

   put_apacket( p);
   //	notify_ready_to_remote();

   return true;
}

void adbhost::process() {
   apacket *p;
   // 1. sync, just internal,  for input/output thread model sync,
   // we just one thread. omit.

   // 2.  sync Handler will send connect message,
   connect(&this->t);
   // receive connect from remote
   if (!handle_connect_response()) {
      return;
   }

   // 3. if we receive connect message from remote, do open
   open_service("shell:cat build.prop");
   //todo:: we should receive : OKAY(send_ready), mean success,
   //                    or CLOSE(send_ready), which means failure.
   if (!handle_open_response()) {
      return;
   }
   handle_command_response();

   // 4. do write what we want to do.

   // 5. close
   close_remote();
}

#if TRACE_PACKETS
#define DUMPMAX 32
void print_packet(const char *label, apacket *p)
{
   char *tag;
   char *x;
   unsigned count;

   switch(p->msg.command){
   case A_SYNC: tag = "SYNC"; break;
   case A_CNXN: tag = "CNXN" ; break;
   case A_OPEN: tag = "OPEN"; break;
   case A_OKAY: tag = "OKAY"; break;
   case A_CLSE: tag = "CLSE"; break;
   case A_WRTE: tag = "WRTE"; break;
   default: tag = "????"; break;
   }

   fprintf(stderr, "%s: %s %08x %08x %04x \"",
           label, tag, p->msg.arg0, p->msg.arg1, p->msg.data_length);
   count = p->msg.data_length;
   x = (char*) p->data;
   if(count > DUMPMAX) {
      count = DUMPMAX;
      tag = "\n";
   } else {
      tag = "\"\n";
   }
   while(count-- > 0){
      if((*x >= ' ') && (*x < 127)) {
         fputc(*x, stderr);
      } else {
         fputc('.', stderr);
      }
      x++;
   }
   fprintf(stderr, tag);
}
#endif
