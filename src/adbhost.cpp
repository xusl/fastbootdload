/*=============================================================================
DESC:
   Multi-Port support Version base on adb protocol.

CHANGE HISTORY:
when        who          what
----------  ---------    --------------------------------------------------------
2013-07-10  shenlong.xu  Init first version

=============================================================================*/


#include "StdAfx.h"

#define   TRACE_TAG  TRACE_ADB
#include "adbhost.h"
#include "adb_auth.h"

#if ADB_TRACE
static void dump_packet( const char*  tag, apacket* const p)
{
   unsigned  command = p->msg.command;
   int       len     = p->msg.data_length;
   char      cmd[5];
   int       n;
   int  nn, len2 = len;
   const unsigned char* ptr = p->data;

   for (n = 0; n < 4; n++) {
      int  b = (command >> (n*8)) & 255;
      if (b >= 32 && b < 127)
         cmd[n] = (char)b;
      else
         cmd[n] = '.';
   }
   cmd[4] = 0;

   DEBUG("%s:  [%08x %s] %08x %08x (%d) ",
     tag, command, cmd, p->msg.arg0, p->msg.arg1, len);

#define DUMP_DATA_LEN 16
   if (len2 > DUMP_DATA_LEN) len2 = DUMP_DATA_LEN;
   char* buffer = (char*)malloc (len2 * sizeof(char) * 2 + 1);

   if (buffer == NULL)
   	return;
   memset(buffer, 0, len2 * sizeof(char) * 2 + 1);


   for (nn = 0; nn < len2; nn++)
      snprintf(buffer+(nn<<1), 2, "%02x", ptr[nn]);
   buffer[2*nn] = '\0';
   DEBUG("%s  ", buffer);

   for (nn = 0; nn < len2; nn++) {
      int  c = ptr[nn];
      if (c < 32 || c > 127)
         c = '.';
      //D("%c", c);
	  snprintf(buffer+nn,1,"%c", c);
   }
   //D("\n");
   buffer[nn] = '\0';
   DEBUG("%s\n", buffer);

   fflush(stdout);
   free(buffer);
}
#endif

adbhost::adbhost(usb_handle *usb)
{
   adbhost(usb, usb_port_address(usb));
}


adbhost::adbhost(usb_handle *usb, unsigned address):
   id(address)
{
   //t = (atransport *)calloc(1, sizeof(atransport));
   memset(&this->t, 0 ,sizeof(atransport));
   init_usb_transport(&this->t, usb, CS_OFFLINE);
   packet_buffer = NULL;
}

adbhost::~adbhost(void)
{
   if (packet_buffer != NULL)
      put_apacket(packet_buffer);

  // kick_transport(&this->t);
   //    transport_unref(&this->t);
   if (this->t.product != NULL)
      free(this->t.product);
}

int adbhost::read_packet( apacket** ppacket)
{
   if(t.read_from_remote(*ppacket, &t) == 0){
      //DEBUG("from_remote: received remote packet, sending to transport %p", t);
   } else {
      ERROR("from_remote: remote read failed for transport %p", *ppacket);
      return -1;
   }

#if ADB_TRACE
   if (ADB_TRACING)
   {
      dump_packet("read_packet", *ppacket);
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
	   dump_packet("write_packet", *ppacket);
   }
#endif

   t.write_to_remote(*ppacket, &t);
   return 0;
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
      ERROR("Transport is null ");
   }

   if(write_packet(&p)){
      ERROR("cannot enqueue packet on transport socket");
   }
}

void adbhost::send_auth_response(UINT8 *token, size_t token_size, atransport *t)
{
    DEBUG("Calling send_auth_response\n");
    apacket *p = get_apacket();
    int ret;

#if 0
    ret = adb_auth_sign(t->key, token, token_size, p->data);
    if (!ret) {
        DEBUG("Error signing the token\n");
        put_apacket(p);
        return;
    }
#endif

    p->msg.command = A_AUTH;
    p->msg.arg0 = ADB_AUTH_SIGNATURE;
    p->msg.data_length = ret;
    send_packet(p, t);
}

void adbhost::send_auth_publickey(atransport *t)
{
    DEBUG("Calling send_auth_publickey\n");
    apacket *p = get_apacket();
    int ret;
#if 0
    ret = adb_auth_get_userkey(p->data, sizeof(p->data));
    if (!ret) {
        DEBUG("Failed to get user public key\n");
        put_apacket(p);
        return;
    }
#endif
    p->msg.command = A_AUTH;
    p->msg.arg0 = ADB_AUTH_RSAPUBLICKEY;
    p->msg.data_length = ret;
    send_packet(p, t);
}

void adbhost::connect(atransport *t)
{
   apacket *cp = get_apacket();
   cp->msg.command = A_CNXN;
   cp->msg.arg0 = A_VERSION;
   cp->msg.arg1 = MAX_PAYLOAD;
   snprintf((char*) cp->data, sizeof cp->data, "%s::", "host" );
   cp->msg.data_length = strlen((char*) cp->data) + 1;
   DEBUG("Calling connect ");
   send_packet(cp, t);

   /* XXX why sleep here? */
   // allow the device some time to respond to the connect message
  // adb_sleep_ms(1000);

   put_apacket(cp);
}


void adbhost::open_service(const char *destination)
{
   apacket *p = get_apacket();
   int len = strlen(destination) + 1;
   DEBUG("Connect_to_remote call ");

   if(len > (MAX_PAYLOAD-1)) {
      ERROR("destination oversized");
   }

   DEBUG("LS(%d): connect('%s')", id, destination);
   p->msg.command = A_OPEN;
   p->msg.arg0 = this->id;
   p->msg.data_length = len;
   strcpy((char*) p->data, destination);
   send_packet(p, &this->t);
   put_apacket(p);
}


int adbhost::enqueue_command(apacket *p)
{
   DEBUG("Calling remote_socket_enqueue");
   p->msg.command = A_WRTE;
   p->msg.arg0 = this->id;
   p->msg.arg1 = this->peer_id;
   p->msg.data_length = p->len;
   send_packet(p, &this->t);
   return 1;
}

void adbhost::notify_ready_to_remote(void)
{
   apacket *p = get_apacket();

   p->msg.command = A_OKAY;
   p->msg.arg0 = this->id;
   p->msg.arg1 = this->peer_id;
   DEBUG("Calling remote_socket_ready");
   send_packet(p, &this->t);
   put_apacket(p);
}

void adbhost::close_remote(void)
{
   apacket *p = get_apacket();

   p->msg.command = A_CLSE;
   p->msg.arg1 = this->id;
   send_packet(p, &this->t);
   DEBUG("RS(%d): closed", this->id);

   for (;;) {
      read_packet(&p);

      if (p->msg.command == A_OKAY) {
         memset(p, 0, sizeof(apacket));
      } else if (p->msg.command == A_CLSE) {
         break;
      } else {
         ERROR("COMMAND ERROR 0x%x", p->msg.command);
      }
   }

   put_apacket(p);
}

bool adbhost::handle_shell_response (void **response, int *len) {
  apacket *p = get_apacket();
  int data_len = 0;
  int ret;
  char *data = (char *)malloc(MAX_PAYLOAD);
  int capacity = MAX_PAYLOAD;
  if (p == NULL || data == NULL) {
    ERROR("No Memory!");
    return false;
  }

  memset(data, 0, MAX_PAYLOAD);

  for (;;) {
    memset(p, 0, sizeof(apacket));
    ret = read_packet(&p);

    if (ret != 0)
        break;

    if (p->msg.command == A_OKAY) {
      INFO("okay response");
    } else if (p->msg.command == A_WRTE) {
      DEBUG("shell return %d bytes data.", p->msg.data_length);

      if (capacity < p->msg.data_length) {
        if (NULL != realloc(data, data_len + capacity + MAX_PAYLOAD)) {
          capacity += MAX_PAYLOAD;
          memset(data + data_len + capacity, 0, MAX_PAYLOAD);
        } else {
          ERROR("No Memory!");
          return false;
        }
      }
      memcpy(data + data_len, p->data, p->msg.data_length);
      data_len += p->msg.data_length;
      capacity -= p->msg.data_length;

      notify_ready_to_remote();
    } else if (p->msg.command == A_CLSE) {
      INFO("shell close");
      notify_ready_to_remote();
      break;
    }
  }

  if (response != NULL && len != NULL) {
    *response = data;
    *len = data_len;
    DEBUG("shell return :%s.", data);
  }

  put_apacket(p);
  return true;
}

bool adbhost::handle_open_response (void) {
   apacket *p =  get_apacket();
   bool result = true;

   if (p == NULL)
      return false;

   read_packet(&p);

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
   } else if (p->msg.command == A_AUTH) {
           if (p->msg.arg0 == ADB_AUTH_TOKEN) {
#if 0
            //t->connection_state = CS_UNAUTHORIZED;
            t->key = adb_auth_nextkey(t->key);
            if (t->key) {
                send_auth_response(p->data, p->msg.data_length, t);
            } else {
                /* No more private keys to try, send the public key */
                send_auth_publickey(t);
            }
#endif
        } else {
          ERROR("Get bad authentication message");
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

   read_packet(&p);

   if (p == NULL || p->msg.command != A_CNXN)
      return false;

   banner = (char*) p->data;

   DEBUG("parse_banner: %s", banner);
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
      DEBUG("setting connection_state to CS_BOOTLOADER\n");
      t.connection_state = CS_BOOTLOADER;
   }
   if(!strcmp(type, "device")) {
      DEBUG("setting connection_state to CS_DEVICE\n");
      t.connection_state = CS_DEVICE;
   }
   if(!strcmp(type, "recovery")) {
      DEBUG("setting connection_state to CS_RECOVERY\n");
      t.connection_state = CS_RECOVERY;
   }


   put_apacket( p);

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
   open_service("shell:ls /");//("shell:cat build.prop");
   //todo:: we should receive : OKAY(send_ready), mean success,
   //                    or CLOSE(send_ready), which means failure.
   if (!handle_open_response()) {
      return;
   }
   // 4. do write what we want to do.
   handle_shell_response(NULL, NULL);
    // 5. close
   //close_remote();



   //need close service now?
   //or do not need close , util connection close?
   connect(&this->t);
   // receive connect from remote
   if (!handle_connect_response()) {
      return;
   }
   do_sync_push("./ReadMe.txt", "/usr");
    close_remote();

   connect(&this->t);
   // receive connect from remote
   if (!handle_connect_response()) {
      return;
   }
   do_sync_pull("/usr/ReadMe.txt", "..");
   // 5. close
   close_remote();

    #if 0
     connect(&this->t);
   // receive connect from remote
   if (!handle_connect_response()) {
      return;
   }

   // 3. if we receive connect message from remote, do open
   open_service("shell:reboot-bootloader");//("shell:cat build.prop");
   //todo:: we should receive : OKAY(send_ready), mean success,
   //                    or CLOSE(send_ready), which means failure.
   if (!handle_open_response()) {
      return;
   }
   // 4. do write what we want to do.
   handle_shell_response();
   #endif
}


int adbhost::shell(const char * command, void **response, int *responselen) {
  char * shell;
  int len;
  if (command == NULL) {
    ERROR("Invalid parameter. command is NULL.");
    return -1;
  }

  connect(&this->t);
  // receive connect from remote
  if (!handle_connect_response()) {
    return -1;
  }

  len = strlen("shell:") + strlen(command) + 1;
  shell = (char *)malloc(len);
  if(shell == 0) {
    ERROR("No memory");
    return -1;
  }
  snprintf(shell, len, "shell:""%s", command);


  // 3. if we receive connect message from remote, do open
  open_service(shell);

  if (!handle_open_response()) {
    free(shell);
    return -1;
  }
  // 4. do write what we want to do.
  handle_shell_response(response, responselen);
  free(shell);
  return 0;
}

int adbhost::reboot_bootloader(MODULE_NAME module_name) {
	if (MODULE_M850==module_name)
	{
		return shell("sys_reboot bootloader", NULL, NULL);
	}
	else
	{
		return shell("reboot-bootloader", NULL, NULL);
	}
}


int adbhost::sync_pull(const char *rpath, const char *lpath) {
  if (rpath == NULL || lpath == NULL) {
    ERROR("Invalid parameter!");
    return -1;
  }

  connect(&this->t);
  if (!handle_connect_response()) {
    return -1;
  }
  do_sync_pull(rpath, lpath);
  close_remote();

  return 0;
}


int adbhost::sync_push(const char *lpath, const char *rpath) {
  if (rpath == NULL || lpath == NULL) {
    ERROR("Invalid parameter!");
    return -1;
  }

  connect(&this->t);
  if (!handle_connect_response()) {
    return -1;
  }
  do_sync_push(lpath, rpath);
  close_remote();

  return 0;
}

int adbhost::do_sync_pull(const char *rpath, const char *lpath)
{
    unsigned mode;
    struct stat st;
	int  tmplen;
	char *tmp;
    int fd = 999;

    open_service("sync:");
    if(!handle_open_response()) {
        ERROR("error: open sync failed");
        return 1;
    }

    if(sync_readmode(fd, rpath, &mode)) {
        return 1;
    }
    if(mode == 0) {
        ERROR("remote object '%s' does not exist", rpath);
        return 1;
    }

    if(S_ISREG(mode) || S_ISLNK(mode) || S_ISCHR(mode) || S_ISBLK(mode)) {
        if(stat(lpath, &st) == 0) {
            if(S_ISDIR(st.st_mode)) {
                    /* if we're copying a remote file to a local directory,
                    ** we *really* want to copy to localdir + "/" + remotefilename
                    */
                const char *name = adb_dirstop(rpath);
                if(name == 0) {
                    name = rpath;
                } else {
                    name++;
                }
                tmplen = strlen(name) + strlen(lpath) + 2;
                tmp = (char *)malloc(tmplen);
                if(tmp == 0) return 1;
                snprintf(tmp, tmplen, "%s/%s", lpath, name);
                lpath = tmp;
            }
        }
        BEGIN();
        if(sync_recv(fd, rpath, lpath)) {
            return 1;
        } else {
            END();
            sync_quit();
            return 0;
        }
    } else if(S_ISDIR(mode)) {
        BEGIN();
        if (copy_remote_dir_local(fd, rpath, lpath, 0)) {
            return 1;
        } else {
            END();
            sync_quit();
            return 0;
        }
    } else {
        ERROR("remote object '%s' not a file or directory", rpath);
        return 1;
    }
}


int adbhost::do_sync_push(const char *lpath, const char *rpath)
{
    struct stat st;
    unsigned mode;
    int fd = 1;
	int  tmplen;
	char *tmp;

    open_service("sync:");
    if(!handle_open_response()) {
        //fprintf(stderr,"error: %s\n", adb_error());
        return 1;
    }

    if(stat(lpath, &st)) {
        ERROR("cannot stat local file '%s'", lpath);
        sync_quit();
        return 1;
    }

    if(S_ISDIR(st.st_mode)) {
        BEGIN();
        if(copy_local_dir_remote(fd, lpath, rpath, 0, 0)) {
            return 1;
        } else {
            END();
            sync_quit();
        }
    } else {
        if(sync_readmode(fd, rpath, &mode)) {
            return 1;
        }
        if((mode != 0) && S_ISDIR(mode)) {
                /* if we're copying a local file to a remote directory,
                ** we *really* want to copy to remotedir + "/" + localfilename
                */
            const char *name = adb_dirstop(lpath);
            if(name == 0) {
                name = lpath;
            } else {
                name++;
            }
            tmplen = strlen(name) + strlen(rpath) + 2;
            tmp = (char *)malloc(strlen(name) + strlen(rpath) + 2);
            if(tmp == 0) return 1;
            snprintf(tmp, tmplen, "%s/%s", rpath, name);
            rpath = tmp;
        }
        BEGIN();
        if(sync_send(fd, lpath, rpath, st.st_mtime, st.st_mode)) {
            return 1;
        } else {
            END();
            sync_quit();
            return 0;
        }
    }

    return 0;
}

int adbhost::sync_send(int fd, const char *lpath, const char *rpath,
                     unsigned mtime, mode_t mode)
{
    syncmsg msg;
    int len, r;
    syncsendbuf *sbuf = &send_buffer;
    char* file_buffer = NULL;
    int size = 0;
    char tmp[64];

    len = strlen(rpath);
    if(len > 1024) goto fail;

    snprintf(tmp, sizeof(tmp), ",%d", mode);
    r = strlen(tmp);

    msg.req.id = ID_SEND;
    msg.req.namelen = htoll(len + r);

    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, rpath, len) || writex(fd, tmp, r)) {
        free(file_buffer);
        goto fail;
    }

    if (file_buffer) {
        write_data_buffer(fd, file_buffer, size, sbuf);
        free(file_buffer);
    } else if (S_ISREG(mode))
        write_data_file(fd, lpath, sbuf);
#ifdef HAVE_SYMLINKS
    else if (S_ISLNK(mode))
        write_data_link(fd, lpath, sbuf);
#endif
    else
        goto fail;

    msg.data.id = ID_DONE;
    msg.data.size = htoll(mtime);
    if(writex(fd, &msg.data, sizeof(msg.data)))
        goto fail;

    if(readx(fd, &msg.status, sizeof(msg.status)))
        return -1;

    if(msg.status.id != ID_OKAY) {
        if(msg.status.id == ID_FAIL) {
            len = ltohl(msg.status.msglen);
            if(len > 256) len = 256;
            if(readx(fd, sbuf->data, len)) {
                return -1;
            }
            sbuf->data[len] = 0;
        } else
            strcpy(sbuf->data, "unknown reason");

        ERROR("failed to copy '%s' to '%s': %s", lpath, rpath, sbuf->data);
        return -1;
    }

    return 0;

fail:
    ERROR("protocol failure");
    return -1;
}


int adbhost::sync_recv(int fd, const char *rpath, const char *lpath)
{
    syncmsg msg;
    int len;
    FILE* lfd = NULL;
    char *buffer = send_buffer.data;
    unsigned id;

    len = strlen(rpath);
    if(len > 1024) return -1;

    msg.req.id = ID_RECV;
    msg.req.namelen = htoll(len);
    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, rpath, len)) {
        return -1;
    }

    if(readx(fd, &msg.data, sizeof(msg.data))) {
        return -1;
    }
    id = msg.data.id;

    if((id == ID_DATA) || (id == ID_DONE)) {
        adb_unlink(lpath);
        mkdirs((char *)lpath);
        lfd = adb_creat(lpath, 0644);
        if(lfd < 0) {
            ERROR("cannot create '%s': %s", lpath, strerror(errno));
            return -1;
        }
        goto handle_data;
    } else {
        goto remote_error;
    }

    for(;;) {
        if(readx(fd, &msg.data, sizeof(msg.data))) {
            return -1;
        }
        id = msg.data.id;

    handle_data:
        len = ltohl(msg.data.size);
        if(id == ID_DONE) break;
        if(id != ID_DATA) goto remote_error;
        if(len > SYNC_DATA_MAX) {
            ERROR("data overrun");
            adb_close(lfd);
            return -1;
        }

        if(readx(fd, buffer, len)) {
            adb_close(lfd);
            return -1;
        }

        if(adb_write(lfd, buffer, len)) {
            ERROR("cannot write '%s': %s", rpath, strerror(errno));
            adb_close(lfd);
            return -1;
        }

        total_bytes += len;
    }

    adb_close(lfd);
    return 0;

remote_error:
    adb_close(lfd);
    adb_unlink(lpath);

    if(id == ID_FAIL) {
        len = ltohl(msg.data.size);
        if(len > 256) len = 256;
        if(readx(fd, buffer, len)) {
            return -1;
        }
        buffer[len] = 0;
    } else {
        memcpy(buffer, &id, 4);
        buffer[4] = 0;
//        strcpy(buffer,"unknown reason");
    }
    ERROR("failed to copy '%s' to '%s': %s", rpath, lpath, buffer);
    return 0;
}

int adbhost::sync_readmode(int fd, const char *path, unsigned *mode)
{
    syncmsg msg;
    int len = strlen(path);

    msg.req.id = ID_STAT;
    msg.req.namelen = htoll(len);

    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, path, len)) {
        return -1;
    }

    if(readx(fd, &msg.stat, sizeof(msg.stat))) {
        return -1;
    }

    if(msg.stat.id != ID_STAT) {
        return -1;
    }

    *mode = ltohl(msg.stat.mode);
    return 0;
}


int adbhost::copy_local_dir_remote(int fd, const char *lpath, const char *rpath,
	int checktimestamps, int listonly)
{
    copyinfo *filelist = 0;
    copyinfo *ci, *next;
    int pushed = 0;
    int skipped = 0;

    if((lpath[0] == 0) || (rpath[0] == 0)) return -1;
    if(lpath[strlen(lpath) - 1] != '/') {
        int  tmplen = strlen(lpath)+2;
        char *tmp = (char *)malloc(tmplen);
        if(tmp == 0) return -1;
        snprintf(tmp, tmplen, "%s/",lpath);
        lpath = tmp;
    }
    if(rpath[strlen(rpath) - 1] != '/') {
        int tmplen = strlen(rpath)+2;
        char *tmp = (char *)malloc(tmplen);
        if(tmp == 0) return -1;
        snprintf(tmp, tmplen, "%s/",rpath);
        rpath = tmp;
    }

    if(local_build_list(&filelist, lpath, rpath)) {
        return -1;
    }

    if(checktimestamps){
        for(ci = filelist; ci != 0; ci = ci->next) {
            if(sync_start_readtime(fd, ci->dst)) {
                return 1;
            }
        }
        for(ci = filelist; ci != 0; ci = ci->next) {
            unsigned int timestamp, mode, size;
            if(sync_finish_readtime(fd, &timestamp, &mode, &size))
                return 1;
            if(size == ci->size) {
                /* for links, we cannot update the atime/mtime */
                if((S_ISREG(ci->mode & mode) && timestamp == ci->time) ||
                    (S_ISLNK(ci->mode & mode) && timestamp >= ci->time))
                    ci->flag = 1;
            }
        }
    }
    for(ci = filelist; ci != 0; ci = next) {
        next = ci->next;
        if(ci->flag == 0) {
            ERROR("%spush: %s -> %s", listonly ? "would " : "", ci->src, ci->dst);
            if(!listonly &&
               sync_send(fd, ci->src, ci->dst, ci->time, ci->mode)){
                return 1;
            }
            pushed++;
        } else {
            skipped++;
        }
        free(ci);
    }

    ERROR("%d file%s pushed. %d file%s skipped.",
            pushed, (pushed == 1) ? "" : "s",
            skipped, (skipped == 1) ? "" : "s");

    return 0;
}


int adbhost::copy_remote_dir_local(int fd, const char *rpath, const char *lpath,
                                 int checktimestamps)
{
    copyinfo *filelist = 0;
    copyinfo *ci, *next;
    int pulled = 0;
    int skipped = 0;

    /* Make sure that both directory paths end in a slash. */
    if (rpath[0] == 0 || lpath[0] == 0) return -1;
    if (rpath[strlen(rpath) - 1] != '/') {
        int  tmplen = strlen(rpath) + 2;
        char *tmp = (char *)malloc(tmplen);
        if (tmp == 0) return -1;
        snprintf(tmp, tmplen, "%s/", rpath);
        rpath = tmp;
    }
    if (lpath[strlen(lpath) - 1] != '/') {
        int  tmplen = strlen(lpath) + 2;
        char *tmp = (char *)malloc(tmplen);
        if (tmp == 0) return -1;
        snprintf(tmp, tmplen, "%s/", lpath);
        lpath = tmp;
    }

    ERROR( "pull: building file list...");
    /* Recursively build the list of files to copy. */
    if (remote_build_list(fd, &filelist, rpath, lpath)) {
        return -1;
    }

#if 0
    if (checktimestamps) {
        for (ci = filelist; ci != 0; ci = ci->next) {
            if (sync_start_readtime(fd, ci->dst)) {
                return 1;
            }
        }
        for (ci = filelist; ci != 0; ci = ci->next) {
            unsigned int timestamp, mode, size;
            if (sync_finish_readtime(fd, &timestamp, &mode, &size))
                return 1;
            if (size == ci->size) {
                /* for links, we cannot update the atime/mtime */
                if ((S_ISREG(ci->mode & mode) && timestamp == ci->time) ||
                    (S_ISLNK(ci->mode & mode) && timestamp >= ci->time))
                    ci->flag = 1;
            }
        }
    }
#endif
    for (ci = filelist; ci != 0; ci = next) {
        next = ci->next;
        if (ci->flag == 0) {
            ERROR( "pull: %s -> %s", ci->src, ci->dst);
            if (sync_recv(fd, ci->src, ci->dst)) {
                return 1;
            }
            pulled++;
        } else {
            skipped++;
        }
        free(ci);
    }

    ERROR( "%d file%s pulled. %d file%s skipped.",
            pulled, (pulled == 1) ? "" : "s",
            skipped, (skipped == 1) ? "" : "s");

    return 0;
}

void
adbhost::sync_ls_build_list_cb(unsigned mode, unsigned size, unsigned time,
                      const char *name, void *cookie)
{
    sync_ls_build_list_cb_args *args = (sync_ls_build_list_cb_args *)cookie;
    copyinfo *ci;

    if (S_ISDIR(mode)) {
        copyinfo **dirlist = args->dirlist;

        /* Don't try recursing down "." or ".." */
        if (name[0] == '.') {
            if (name[1] == '\0') return;
            if ((name[1] == '.') && (name[2] == '\0')) return;
        }

        ci = mkcopyinfo(args->rpath, args->lpath, name, 1);
        ci->next = *dirlist;
        *dirlist = ci;
    } else if (S_ISREG(mode) || S_ISLNK(mode)) {
        copyinfo **filelist = args->filelist;

        ci = mkcopyinfo(args->rpath, args->lpath, name, 0);
        ci->time = time;
        ci->mode = mode;
        ci->size = size;
        ci->next = *filelist;
        *filelist = ci;
    } else {
        ERROR( "skipping special file '%s'", name);
    }
}

int adbhost::remote_build_list(int syncfd, copyinfo **filelist,
                             const char *rpath, const char *lpath)
{
    copyinfo *dirlist = NULL;
    sync_ls_build_list_cb_args args;

    args.filelist = filelist;
    args.dirlist = &dirlist;
    args.rpath = rpath;
    args.lpath = lpath;

    /* Put the files/dirs in rpath on the lists. */
    if (sync_ls(syncfd, rpath, //sync_ls_build_list_cb,
		(void *)&args)) {
        return 1;
    }

    /* Recurse into each directory we found. */
    while (dirlist != NULL) {
        copyinfo *next = dirlist->next;
        if (remote_build_list(syncfd, filelist, dirlist->src, dirlist->dst)) {
            return 1;
        }
        free(dirlist);
        dirlist = next;
    }

    return 0;
}

int adbhost::sync_ls(int fd, const char *path, //sync_ls_cb func,
	void *cookie)
{
    syncmsg msg;
    char buf[257];
    int len;

    len = strlen(path);
    if(len > 1024) goto fail;

    msg.req.id = ID_LIST;
    msg.req.namelen = htoll(len);

    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, path, len)) {
        goto fail;
    }

    for(;;) {
        if(readx(fd, &msg.dent, sizeof(msg.dent))) break;
        if(msg.dent.id == ID_DONE) return 0;
        if(msg.dent.id != ID_DENT) break;

        len = ltohl(msg.dent.namelen);
        if(len > 256) break;

        if(readx(fd, buf, len)) break;
        buf[len] = 0;

       // func TODO::
		sync_ls_build_list_cb(ltohl(msg.dent.mode),
             ltohl(msg.dent.size),
             ltohl(msg.dent.time),
             buf, cookie);
    }

fail:
//    adb_close(fd);
    return -1;
}

int adbhost::write_data_file(int fd, const char *path, syncsendbuf *sbuf)
{
    int err = 0;
	FILE* lfd;

    lfd = adb_open(path, "rb");
    if(lfd < 0) {
        ERROR("cannot open '%s': %s", path, strerror(errno));
        return -1;
    }

    sbuf->id = ID_DATA;
    for(;;) {
        int ret;

        ret = adb_read(lfd, sbuf->data, SYNC_DATA_MAX);
        if(!ret)
            break;

        if(ret < 0) {
            if(errno == EINTR)
                continue;
            ERROR("cannot read '%s': %s", path, strerror(errno));
            break;
        }

        sbuf->size = htoll(ret);
        if(writex(fd, sbuf, sizeof(unsigned) * 2 + ret)){
            err = -1;
            break;
        }
        total_bytes += ret;
    }

    adb_close(lfd);
    return err;
}

int adbhost::write_data_buffer(int fd, char* file_buffer, int size, syncsendbuf *sbuf)
{
    int err = 0;
    int total = 0;

    sbuf->id = ID_DATA;
    while (total < size) {
        int count = size - total;
        if (count > SYNC_DATA_MAX) {
            count = SYNC_DATA_MAX;
        }

        memcpy(sbuf->data, &file_buffer[total], count);
        sbuf->size = htoll(count);
        if(writex(fd, sbuf, sizeof(unsigned) * 2 + count)){
            err = -1;
            break;
        }
        total += count;
        total_bytes += count;
    }

    return err;
}

void adbhost::sync_quit(void)
{
    syncmsg msg;

    msg.req.id = ID_QUIT;
    msg.req.namelen = 0;

    writex(0, &msg.req, sizeof(msg.req));
}

int adbhost::readx(int fd, void *ptr, size_t len) {
   apacket *p = NULL;

   if (packet_buffer == NULL)
   {
      p = get_apacket();
      for (;;)
      {
         read_packet(&p);

         if (p->msg.command == A_OKAY) {
            memset(p, 0, sizeof(apacket));
         } else if (p->msg.command == A_WRTE) {
            notify_ready_to_remote();
            break;
         } else {
            ERROR("COMMAND ERROR");
            return 1;
         }
      }
   } else {
      p = packet_buffer;
      packet_buffer = NULL;
   }

   if (len > p->msg.data_length) {
      ERROR("ERROR");
      return 1;
   }
   memcpy(ptr, p->data, len);

   if (len < p->msg.data_length)
   {
      packet_buffer = get_apacket();
      packet_buffer->msg.command = p->msg.command;
      packet_buffer->msg.data_length = p->msg.data_length - len;
      memcpy(packet_buffer->data, p->data + len, packet_buffer->msg.data_length);
   }

   put_apacket(p);
   return 0;
}

int adbhost::writex(int fd, const void *ptr, size_t len){
   apacket *p = get_apacket();
   p->len = len;
   memcpy(p->data, ptr, len);
   enqueue_command(p);
   put_apacket(p);
   return 0;
}
int adbhost::local_build_list(copyinfo **filelist,
                            const char *lpath, const char *rpath)
{
   DIR *d;
   struct dirent *de;
   struct stat st;
   copyinfo *dirlist = 0;
   copyinfo *ci, *next;

   //    fprintf(stderr,"local_build_list('%s','%s')\n", lpath, rpath);

   d = opendir(lpath);
   if(d == 0) {
      ERROR("cannot open '%s': %s", lpath, strerror(errno));
      return -1;
   }

   while((de = readdir(d))) {
      char stat_path[PATH_MAX];
      char *name = de->d_name;

      if(name[0] == '.') {
         if(name[1] == 0) continue;
         if((name[1] == '.') && (name[2] == 0)) continue;
      }

      /*
       * We could use d_type if HAVE_DIRENT_D_TYPE is defined, but reiserfs
       * always returns DT_UNKNOWN, so we just use stat() for all cases.
       */
      if (strlen(lpath) + strlen(de->d_name) + 1 > sizeof(stat_path))
         continue;
      strcpy(stat_path, lpath);
      strcat(stat_path, de->d_name);
      stat(stat_path, &st);

      if (S_ISDIR(st.st_mode)) {
         ci = mkcopyinfo(lpath, rpath, name, 1);
         ci->next = dirlist;
         dirlist = ci;
      } else {
         ci = mkcopyinfo(lpath, rpath, name, 0);
         if(lstat(ci->src, &st)) {
            closedir(d);
            ERROR("cannot stat '%s': %s", ci->src, strerror(errno));
            return -1;
         }
         if(!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)) {
            ERROR( "skipping special file '%s'", ci->src);
            free(ci);
         } else {
            ci->time = st.st_mtime;
            ci->mode = st.st_mode;
            ci->size = st.st_size;
            ci->next = *filelist;
            *filelist = ci;
         }
      }
   }

   closedir(d);

   for(ci = dirlist; ci != 0; ci = next) {
      next = ci->next;
      local_build_list(filelist, ci->src, ci->dst);
      free(ci);
   }

   return 0;
}


int adbhost::adb_read(FILE* fd, void *ptr, size_t len)
{
    int r;

    if (feof(fd)) {
        DEBUG("adb_read: reach end of file.", fd);
        return 0;
    }

    DEBUG("adb_read: %d %p %d", fd, ptr, (int)len);
    r = fread(ptr, 1, len, fd);

    if(r < 0) {
        DEBUG("adb_read: %d %d , errno :%s, ferror %d", fd, r, strerror(errno), ferror(fd));
        return -1;
    }

    return r;
}

int adbhost::adb_write(FILE* fd, const void *ptr, size_t len)
{
    char *p = (char*) ptr;
    int r;

    while(len > 0) {
        r = fwrite(p, 1, len, fd);
        if(r > 0) {
            len -= r;
            p += r;
        } else {
            DEBUG("adb_write: %d %d %s", fd, r, strerror(errno));
            if((r < 0) && (errno == EINTR)) continue;
            return -1;
        }
    }

    DEBUG("adb_write: %d ok", fd);
    return 0;
}

FILE* adbhost::adb_open(const char *path, const char *mode){
	return fopen(path, mode);

}
 FILE* adbhost::adb_creat(const char*  path, int  mode)
{
 	return fopen(path, "w+");
}

int adbhost::adb_close(FILE* fd) {
	return fclose(fd);
}

copyinfo *adbhost::mkcopyinfo(const char *spath, const char *dpath,
                     const char *name, int isdir)
{
    int slen = strlen(spath);
    int dlen = strlen(dpath);
    int nlen = strlen(name);
    int ssize = slen + nlen + 2;
    int dsize = dlen + nlen + 2;

    copyinfo *ci = (copyinfo *)malloc(sizeof(copyinfo) + ssize + dsize);
    if(ci == 0) {
        ERROR("out of memory");
        abort();
    }

    ci->next = 0;
    ci->time = 0;
    ci->mode = 0;
    ci->size = 0;
    ci->flag = 0;
    ci->src = (const char*)(ci + 1);
    ci->dst = ci->src + ssize;
    snprintf((char*) ci->src, ssize, isdir ? "%s%s/" : "%s%s", spath, name);
    snprintf((char*) ci->dst, dsize, isdir ? "%s%s/" : "%s%s", dpath, name);

//    fprintf(stderr,"mkcopyinfo('%s','%s')\n", ci->src, ci->dst);
    return ci;
}

int adbhost::mkdirs(char *name)
{
    int ret;
    char *x = name + 1;

    for(;;) {
        x = (char *)adb_dirstart(x);
        if(x == 0) return 0;
        *x = 0;
        ret = adb_mkdir(name, 0775);
        *x = OS_PATH_SEPARATOR;
        if((ret < 0) && (errno != EEXIST)) {
            return ret;
        }
        x++;
    }
    return 0;
}


int adbhost::sync_start_readtime(int fd, const char *path)
{
    syncmsg msg;
    int len = strlen(path);

    msg.req.id = ID_STAT;
    msg.req.namelen = htoll(len);

    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, path, len)) {
        return -1;
    }

    return 0;
}

int adbhost::sync_finish_readtime(int fd, unsigned int *timestamp,
                                unsigned int *mode, unsigned int *size)
{
    syncmsg msg;

    if(readx(fd, &msg.stat, sizeof(msg.stat)))
        return -1;

    if(msg.stat.id != ID_STAT)
        return -1;

    *timestamp = ltohl(msg.stat.time);
    *mode = ltohl(msg.stat.mode);
    *size = ltohl(msg.stat.size);

    return 0;
}

 void adbhost::BEGIN()
{
    total_bytes = 0;
    start_time = now();
}

 void adbhost::END()
{
    long  t = now() - start_time;
    if(total_bytes == 0) return;

    if (t == 0)  /* prevent division by 0 :-) */
        t = 1000000;

    ERROR("%lld KB/s (%d bytes in %lld.%03llds)",
            ((((long ) total_bytes) * 1000000LL) / t) / 1024LL,
            total_bytes, (t / 1000000LL), (t % 1000000LL) / 1000LL);
}

apacket *get_apacket(void)
{
    apacket *p = (apacket *)malloc(sizeof(apacket));
    if(p == 0) ERROR("failed to allocate an apacket");
    memset(p, 0, sizeof(apacket) - MAX_PAYLOAD);
    return p;
}

void put_apacket(apacket *p)
{
    free(p);
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

   ERROR( "%s: %s %08x %08x %04x \"",
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
   ERROR( tag);
}
#endif
