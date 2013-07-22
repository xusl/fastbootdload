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
#include "dirent.h"
#include "file_sync_service.h"
#include "adb.h"
#include "stdio.h"

typedef unsigned short mode_t;

//typedef struct copyinfo copyinfo;
 struct copyinfo
{
    //copyinfo *next; //vc not support
	 struct copyinfo  *next;
    const char *src;
    const char *dst;
    unsigned int time;
    unsigned int mode;
    unsigned int size;
    int flag;
    //char data[0];
};

typedef struct copyinfo copyinfo;


typedef struct {
    copyinfo **filelist;
    copyinfo **dirlist;
    const char *rpath;
    const char *lpath;
} sync_ls_build_list_cb_args;

class adbhost
{
	public:
		adbhost(void);
		adbhost(usb_handle *usb, unsigned address);
		~adbhost(void);

		void process(void);

	private:
		void open_service(const char *destination);
		void connect(atransport *t);

		int read_packet(apacket** ppacket);
		int write_packet(apacket** ppacket);
		void send_packet(apacket *p, atransport *t);

		bool handle_connect_response(void);
		bool handle_open_response(void);
		bool handle_command_response (void);

		int do_sync_pull(const char *rpath, const char *lpath);
		int do_sync_push(const char *lpath, const char *rpath);
		void sync_quit(void);

		int sync_send(int fd, const char *lpath, const char *rpath,
                     unsigned mtime, mode_t mode);
		int sync_recv(int fd, const char *rpath, const char *lpath);
		int sync_readmode(int fd, const char *path, unsigned *mode);
		int copy_remote_dir_local(int fd, const char *rpath, const char *lpath,
                                 int checktimestamps);
		 int copy_local_dir_remote(int fd, const char *lpath, const char *rpath,
		 	                           int checktimestamps, int listonly);

		int write_data_file(int fd, const char *path, syncsendbuf *sbuf);
		int write_data_buffer(int fd, char* file_buffer, int size, syncsendbuf *sbuf);
		int remote_build_list(int syncfd, copyinfo **filelist,
                             const char *rpath, const char *lpath);
		void
		sync_ls_build_list_cb(unsigned mode, unsigned size, unsigned time,
                      const char *name, void *cookie);
		//typedef void (*sync_ls_cb)(unsigned mode, unsigned size, unsigned time,
		//			const char *name, void *cookie);
		int sync_ls(int fd, const char *path, //sync_ls_cb func,
							void *cookie);
		int local_build_list(copyinfo **filelist,
                            const char *lpath, const char *rpath);
		copyinfo *mkcopyinfo(const char *spath, const char *dpath,
                     const char *name, int isdir);
		int writex(int fd, const void *ptr, size_t len);
		int readx(int fd, void *ptr, size_t len);


	 FILE*  adb_open(const char *path, const char *mode="rw");
	 FILE*  adb_creat(const char*  path, int  mode);
	 int  adb_read(FILE* fd, void* buf, size_t len);
	 int  adb_write(FILE* fd, const void*  buf, size_t  len);
	 //int  adb_lseek(int  fd, int  pos, int  where);
	 //int  adb_shutdown(int  fd);
	 int  adb_close(FILE* fd);

		int mkdirs(char *name);
		int sync_start_readtime(int fd, const char *path);
		int sync_finish_readtime(int fd, unsigned int *timestamp,
                                unsigned int *mode, unsigned int *size);
    void BEGIN(void);
		void END(void);

 	  void notify_ready_to_remote(void);
		void close_remote(void);
		int enqueue_command(apacket *p);

	private:
		atransport t;
		unsigned id; //local id, address
		unsigned peer_id;

		unsigned total_bytes;
    long start_time;
	  syncsendbuf send_buffer;
	  apacket *packet_buffer;
};
