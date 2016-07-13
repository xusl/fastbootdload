#include "StdAfx.h"
#include "telnet.h"
#include "arpa_telnet.h"
#include "log.h"
#include "stdio.h"

#define elapsed_ms  (int)curlx_tvdiff(curlx_tvnow(), initial_tv)

#define CURL_SB_CLEAR(x)  x->subpointer = x->subbuffer;
#define CURL_SB_TERM(x)   { x->subend = x->subpointer; CURL_SB_CLEAR(x); }
#define CURL_SB_ACCUM(x,c) \
  if(x->subpointer < (x->subbuffer+sizeof x->subbuffer)) { \
    *x->subpointer++ = (c); \
  }

#define  CURL_SB_GET(x) ((*x->subpointer++)&0xff)
#define  CURL_SB_PEEK(x)   ((*x->subpointer)&0xff)
#define  CURL_SB_EOF(x) (x->subpointer >= x->subend)
#define  CURL_SB_LEN(x) (x->subend - x->subpointer)


telnet::~telnet() {
  curl_slist_free_all(telnet_vars);
  telnet_vars = NULL;
}

void telnet::negotiate() {
  int i;
  //struct TELNET *tn = (struct TELNET *) conn->data->state.proto.telnet;
    telnet *tn = this;
  for(i = 0;i < CURL_NTELOPTS;i++) {
    if(tn->us_preferred[i] == CURL_YES)
      set_local_option(i, CURL_YES);

    if(tn->him_preferred[i] == CURL_YES)
      set_remote_option(i, CURL_YES);
  }
}

void telnet::send_negotiation(int cmd, int option) {
   unsigned char buf[3];
   ssize_t bytes_written;
   int err;

   buf[0] = CURL_IAC;
   buf[1] = (unsigned char)cmd;
   buf[2] = (unsigned char)option;

   bytes_written = swrite(sockfd, buf, 3);
   if(bytes_written < 0) {
     err = SOCKERRNO;
     LOGE("Sending data failed (%d)",err);
   }

   printoption("SENT", cmd, option);    ;
}

void telnet::printoption(const char *direction, int cmd, int option)
{
  const char *fmt;
  const char *opt;

  if(verbose) {
    if(cmd == CURL_IAC) {
      if(CURL_TELCMD_OK(option))
        LOGD( "%s IAC %s", direction, CURL_TELCMD(option));
      else
        LOGD( "%s IAC %d", direction, option);
    }
    else {
      fmt = (cmd == CURL_WILL) ? "WILL" : (cmd == CURL_WONT) ? "WONT" :
        (cmd == CURL_DO) ? "DO" : (cmd == CURL_DONT) ? "DONT" : 0;
      if(fmt) {
        if(CURL_TELOPT_OK(option))
          opt = CURL_TELOPT(option);
        else if(option == CURL_TELOPT_EXOPL)
          opt = "EXOPL";
        else
          opt = NULL;

        if(opt)
          LOGD( "%s %s %s", direction, fmt, opt);
        else
          LOGD( "%s %s %d", direction, fmt, option);
      }
      else
        LOGD( "%s %d %d", direction, cmd, option);
    }
  }
}


void telnet::set_remote_option(int option, int newstate) {
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet *tn = this;
  if(newstate == CURL_YES) {
    switch(tn->him[option]) {
    case CURL_NO:
      tn->him[option] = CURL_WANTYES;
      send_negotiation(CURL_DO, option);
      break;

    case CURL_YES:
      /* Already enabled */
      break;

    case CURL_WANTNO:
      switch(tn->himq[option]) {
      case CURL_EMPTY:
        /* Already negotiating for CURL_YES, queue the request */
        tn->himq[option] = CURL_OPPOSITE;
        break;
      case CURL_OPPOSITE:
        /* Error: already queued an enable request */
        break;
      }
      break;

    case CURL_WANTYES:
      switch(tn->himq[option]) {
      case CURL_EMPTY:
        /* Error: already negotiating for enable */
        break;
      case CURL_OPPOSITE:
        tn->himq[option] = CURL_EMPTY;
        break;
      }
      break;
    }
  }
  else { /* NO */
    switch(tn->him[option]) {
    case CURL_NO:
      /* Already disabled */
      break;

    case CURL_YES:
      tn->him[option] = CURL_WANTNO;
      send_negotiation(CURL_DONT, option);
      break;

    case CURL_WANTNO:
      switch(tn->himq[option]) {
      case CURL_EMPTY:
        /* Already negotiating for NO */
        break;
      case CURL_OPPOSITE:
        tn->himq[option] = CURL_EMPTY;
        break;
      }
      break;

    case CURL_WANTYES:
      switch(tn->himq[option]) {
      case CURL_EMPTY:
        tn->himq[option] = CURL_OPPOSITE;
        break;
      case CURL_OPPOSITE:
        break;
      }
      break;
    }
  }
}


void telnet::rec_will( int option)
{
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet *tn = this;
  LOGD("ENTER");
  switch(tn->him[option]) {
  case CURL_NO:
    if(tn->him_preferred[option] == CURL_YES) {
      tn->him[option] = CURL_YES;
      send_negotiation(CURL_DO, option);
    }
    else
      send_negotiation(CURL_DONT, option);

    break;

  case CURL_YES:
    /* Already enabled */
    break;

  case CURL_WANTNO:
    switch(tn->himq[option]) {
    case CURL_EMPTY:
      /* Error: DONT answered by WILL */
      tn->him[option] = CURL_NO;
      break;
    case CURL_OPPOSITE:
      /* Error: DONT answered by WILL */
      tn->him[option] = CURL_YES;
      tn->himq[option] = CURL_EMPTY;
      break;
    }
    break;

  case CURL_WANTYES:
    switch(tn->himq[option]) {
    case CURL_EMPTY:
      tn->him[option] = CURL_YES;
      break;
    case CURL_OPPOSITE:
      tn->him[option] = CURL_WANTNO;
      tn->himq[option] = CURL_EMPTY;
      send_negotiation(CURL_DONT, option);
      break;
    }
    break;
  }
}

void telnet::rec_wont( int option)
{
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet *tn = this;
  LOGD("ENTER");
  switch(tn->him[option]) {
  case CURL_NO:
    /* Already disabled */
    break;

  case CURL_YES:
    tn->him[option] = CURL_NO;
    send_negotiation(CURL_DONT, option);
    break;

  case CURL_WANTNO:
    switch(tn->himq[option]) {
    case CURL_EMPTY:
      tn->him[option] = CURL_NO;
      break;

    case CURL_OPPOSITE:
      tn->him[option] = CURL_WANTYES;
      tn->himq[option] = CURL_EMPTY;
      send_negotiation(CURL_DO, option);
      break;
    }
    break;

  case CURL_WANTYES:
    switch(tn->himq[option]) {
    case CURL_EMPTY:
      tn->him[option] = CURL_NO;
      break;
    case CURL_OPPOSITE:
      tn->him[option] = CURL_NO;
      tn->himq[option] = CURL_EMPTY;
      break;
    }
    break;
  }
}

void telnet::set_local_option( int option, int newstate)
{
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet *tn = this;
  LOGD("ENTER");
  if(newstate == CURL_YES) {
    switch(tn->us[option]) {
    case CURL_NO:
      tn->us[option] = CURL_WANTYES;
      send_negotiation(CURL_WILL, option);
      break;

    case CURL_YES:
      /* Already enabled */
      break;

    case CURL_WANTNO:
      switch(tn->usq[option]) {
      case CURL_EMPTY:
        /* Already negotiating for CURL_YES, queue the request */
        tn->usq[option] = CURL_OPPOSITE;
        break;
      case CURL_OPPOSITE:
        /* Error: already queued an enable request */
        break;
      }
      break;

    case CURL_WANTYES:
      switch(tn->usq[option]) {
      case CURL_EMPTY:
        /* Error: already negotiating for enable */
        break;
      case CURL_OPPOSITE:
        tn->usq[option] = CURL_EMPTY;
        break;
      }
      break;
    }
  }
  else { /* NO */
    switch(tn->us[option]) {
    case CURL_NO:
      /* Already disabled */
      break;

    case CURL_YES:
      tn->us[option] = CURL_WANTNO;
      send_negotiation(CURL_WONT, option);
      break;

    case CURL_WANTNO:
      switch(tn->usq[option]) {
      case CURL_EMPTY:
        /* Already negotiating for NO */
        break;
      case CURL_OPPOSITE:
        tn->usq[option] = CURL_EMPTY;
        break;
      }
      break;

    case CURL_WANTYES:
      switch(tn->usq[option]) {
      case CURL_EMPTY:
        tn->usq[option] = CURL_OPPOSITE;
        break;
      case CURL_OPPOSITE:
        break;
      }
      break;
    }
  }
}

void telnet::rec_do( int option)
{
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet *tn = this;
  LOGD("ENTER");
  switch(tn->us[option]) {
  case CURL_NO:
    if(tn->us_preferred[option] == CURL_YES) {
      tn->us[option] = CURL_YES;
      send_negotiation(CURL_WILL, option);
    }
    else
      send_negotiation(CURL_WONT, option);
    break;

  case CURL_YES:
    /* Already enabled */
    break;

  case CURL_WANTNO:
    switch(tn->usq[option]) {
    case CURL_EMPTY:
      /* Error: DONT answered by WILL */
      tn->us[option] = CURL_NO;
      break;
    case CURL_OPPOSITE:
      /* Error: DONT answered by WILL */
      tn->us[option] = CURL_YES;
      tn->usq[option] = CURL_EMPTY;
      break;
    }
    break;

  case CURL_WANTYES:
    switch(tn->usq[option]) {
    case CURL_EMPTY:
      tn->us[option] = CURL_YES;
      break;
    case CURL_OPPOSITE:
      tn->us[option] = CURL_WANTNO;
      tn->himq[option] = CURL_EMPTY;
      send_negotiation(CURL_WONT, option);
      break;
    }
    break;
  }
}

void telnet::rec_dont( int option)
{
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet *tn = this;
  LOGD("ENTER");
  switch(tn->us[option]) {
  case CURL_NO:
    /* Already disabled */
    break;

  case CURL_YES:
    tn->us[option] = CURL_NO;
    send_negotiation(CURL_WONT, option);
    break;

  case CURL_WANTNO:
    switch(tn->usq[option]) {
    case CURL_EMPTY:
      tn->us[option] = CURL_NO;
      break;

    case CURL_OPPOSITE:
      tn->us[option] = CURL_WANTYES;
      tn->usq[option] = CURL_EMPTY;
      send_negotiation(CURL_WILL, option);
      break;
    }
    break;

  case CURL_WANTYES:
    switch(tn->usq[option]) {
    case CURL_EMPTY:
      tn->us[option] = CURL_NO;
      break;
    case CURL_OPPOSITE:
      tn->us[option] = CURL_NO;
      tn->usq[option] = CURL_EMPTY;
      break;
    }
    break;
  }
}
void telnet::printsub(int direction, unsigned char *pointer,
         size_t length) {
  unsigned int i = 0;

  if(verbose) {
    if(direction) {
      LOGD( "%s IAC SB ", (direction == '<')? "RCVD":"SENT");
      if(length >= 3) {
        int j;

        i = pointer[length-2];
        j = pointer[length-1];

        if(i != CURL_IAC || j != CURL_SE) {
          LOGD( "(terminated by ");
          if(CURL_TELOPT_OK(i))
            LOGD( "%s ", CURL_TELOPT(i));
          else if(CURL_TELCMD_OK(i))
            LOGD( "%s ", CURL_TELCMD(i));
          else
            LOGD( "%u ", i);
          if(CURL_TELOPT_OK(j))
            LOGD( "%s", CURL_TELOPT(j));
          else if(CURL_TELCMD_OK(j))
            LOGD( "%s", CURL_TELCMD(j));
          else
            LOGD( "%d", j);
          LOGD( ", not IAC SE!) ");
        }
      }
      length -= 2;
    }
    if(length < 1) {
      LOGD( "(Empty suboption?)");
      return;
    }

    if(CURL_TELOPT_OK(pointer[0])) {
      switch(pointer[0]) {
        case CURL_TELOPT_TTYPE:
        case CURL_TELOPT_XDISPLOC:
        case CURL_TELOPT_NEW_ENVIRON:
          LOGD( "%s", CURL_TELOPT(pointer[0]));
          break;
        default:
          LOGD( "%s (unsupported)", CURL_TELOPT(pointer[0]));
          break;
      }
    }
    else
      LOGD( "%d (unknown)", pointer[i]);

    switch(pointer[1]) {
      case CURL_TELQUAL_IS:
        LOGD( " IS");
        break;
      case CURL_TELQUAL_SEND:
        LOGD( " SEND");
        break;
      case CURL_TELQUAL_INFO:
        LOGD( " INFO/REPLY");
        break;
      case CURL_TELQUAL_NAME:
        LOGD( " NAME");
        break;
    }

    switch(pointer[0]) {
      case CURL_TELOPT_TTYPE:
      case CURL_TELOPT_XDISPLOC:
        pointer[length] = 0;
        LOGD( " \"%s\"", &pointer[2]);
        break;
      case CURL_TELOPT_NEW_ENVIRON:
        if(pointer[1] == CURL_TELQUAL_IS) {
          LOGD( " ");
          for(i = 3;i < length;i++) {
            switch(pointer[i]) {
              case CURL_NEW_ENV_VAR:
                LOGD( ", ");
                break;
              case CURL_NEW_ENV_VALUE:
                LOGD( " = ");
                break;
              default:
                LOGD( "%c", pointer[i]);
                break;
            }
          }
        }
        break;
      default:
        for(i = 2; i < length; i++)
          LOGD( " %.2x", pointer[i]);
        break;
    }

    if(direction)
      LOGD( "\n");
  }
}

CURLcode telnet::check_telnet_options()
{
  CURLcode result = CURLE_OK;
#if 0
  struct curl_slist *head;
  struct curl_slist *beg;
  char option_keyword[128];
  char option_arg[256];
  //struct SessionHandle *data = conn->data;
  //struct TELNET *tn = (struct TELNET *)conn->data->state.proto.telnet;
  telnet * tn = this;
  /* Add the user name as an environment variable if it
     was given on the command line */
  if(conn->bits.user_passwd) {
    snprintf(option_arg, sizeof(option_arg), "USER,%s", conn->user);
    beg = curl_slist_append(tn->telnet_vars, option_arg);
    if(!beg) {
      curl_slist_free_all(tn->telnet_vars);
      tn->telnet_vars = NULL;
      return CURLE_OUT_OF_MEMORY;
    }
    tn->telnet_vars = beg;
    tn->us_preferred[CURL_TELOPT_NEW_ENVIRON] = CURL_YES;
  }

  for(head = data->set.telnet_options; head; head=head->next) {
    if(sscanf(head->data, "%127[^= ]%*[ =]%255s",
              option_keyword, option_arg) == 2) {

      /* Terminal type */
      if(Curl_raw_equal(option_keyword, "TTYPE")) {
        strncpy(tn->subopt_ttype, option_arg, 31);
        tn->subopt_ttype[31] = 0; /* String termination */
        tn->us_preferred[CURL_TELOPT_TTYPE] = CURL_YES;
        continue;
      }

      /* Display variable */
      if(Curl_raw_equal(option_keyword, "XDISPLOC")) {
        strncpy(tn->subopt_xdisploc, option_arg, 127);
        tn->subopt_xdisploc[127] = 0; /* String termination */
        tn->us_preferred[CURL_TELOPT_XDISPLOC] = CURL_YES;
        continue;
      }

      /* Environment variable */
      if(Curl_raw_equal(option_keyword, "NEW_ENV")) {
        beg = curl_slist_append(tn->telnet_vars, option_arg);
        if(!beg) {
          result = CURLE_OUT_OF_MEMORY;
          break;
        }
        tn->telnet_vars = beg;
        tn->us_preferred[CURL_TELOPT_NEW_ENVIRON] = CURL_YES;
        continue;
      }

      LOGD( "Unknown telnet option %s", head->data);
      result = CURLE_UNKNOWN_TELNET_OPTION;
      break;
    }
    else {
      LOGD( "Syntax error in telnet option: %s", head->data);
      result = CURLE_TELNET_OPTION_SYNTAX;
      break;
    }
  }

  if(result) {
    curl_slist_free_all(tn->telnet_vars);
    tn->telnet_vars = NULL;
  }
#endif
  return result;
}

void telnet::suboption() {
  struct curl_slist *v;
  unsigned char temp[2048];
  ssize_t bytes_written;
  size_t len;
  size_t tmplen;
  int err;
  char varname[128];
  char varval[128];
  //struct SessionHandle *data = conn->data;
  //struct TELNET *tn = (struct TELNET *)data->state.proto.telnet;
  telnet *tn = this;

  printsub('<', (unsigned char *)tn->subbuffer, CURL_SB_LEN(tn)+2);
  switch (CURL_SB_GET(tn)) {
    case CURL_TELOPT_TTYPE:
      len = strlen(tn->subopt_ttype) + 4 + 2;
      _snprintf((char *)temp, sizeof(temp),
               "%c%c%c%c%s%c%c", CURL_IAC, CURL_SB, CURL_TELOPT_TTYPE,
               CURL_TELQUAL_IS, tn->subopt_ttype, CURL_IAC, CURL_SE);
      bytes_written = swrite(sockfd, temp, len);
      if(bytes_written < 0) {
        err = SOCKERRNO;
        LOGD("Sending data failed (%d)",err);
      }
      printsub('>', &temp[2], len-2);
      break;
    case CURL_TELOPT_XDISPLOC:
      len = strlen(tn->subopt_xdisploc) + 4 + 2;
      _snprintf((char *)temp, sizeof(temp),
               "%c%c%c%c%s%c%c", CURL_IAC, CURL_SB, CURL_TELOPT_XDISPLOC,
               CURL_TELQUAL_IS, tn->subopt_xdisploc, CURL_IAC, CURL_SE);
      bytes_written = swrite(sockfd, temp, len);
      if(bytes_written < 0) {
        err = SOCKERRNO;
        LOGD("Sending data failed (%d)",err);
      }
      printsub('>', &temp[2], len-2);
      break;
    case CURL_TELOPT_NEW_ENVIRON:
      _snprintf((char *)temp, sizeof(temp),
               "%c%c%c%c", CURL_IAC, CURL_SB, CURL_TELOPT_NEW_ENVIRON,
               CURL_TELQUAL_IS);
      len = 4;

      for(v = tn->telnet_vars;v;v = v->next) {
        tmplen = (strlen(v->data) + 1);
        /* Add the variable only if it fits */
        if(len + tmplen < (int)sizeof(temp)-6) {
          sscanf(v->data, "%127[^,],%127s", varname, varval);
          _snprintf((char *)&temp[len], sizeof(temp) - len,
                   "%c%s%c%s", CURL_NEW_ENV_VAR, varname,
                   CURL_NEW_ENV_VALUE, varval);
          len += tmplen;
        }
      }
      _snprintf((char *)&temp[len], sizeof(temp) - len,
               "%c%c", CURL_IAC, CURL_SE);
      len += 2;
      bytes_written = swrite(sockfd, temp, len);
      if(bytes_written < 0) {
        err = SOCKERRNO;
        LOGD("Sending data failed (%d)",err);
      }
      printsub('>', &temp[2], len-2);
      break;
  }
  return;    ;
}

CURLcode telnet::telrcv(const unsigned char *inbuf, /* Data received from socket */
                ssize_t count)              /* Number of bytes received */
{
  unsigned char c;
  CURLcode result;
  int in = 0;
  int startwrite=-1;
  //struct SessionHandle *data = conn->data;
  //struct TELNET *tn = (struct TELNET *)data->state.proto.telnet;
  telnet* tn = this;
#if 0
#define startskipping()                                       \
  if(startwrite >= 0) {                                       \
    result = Curl_client_write(conn,                          \
                               CLIENTWRITE_BODY,              \
                               (char *)&inbuf[startwrite],    \
                               in-startwrite);                \
    if(result != CURLE_OK)                                    \
      return result;                                          \
  }                                                           \
  startwrite = -1
#else
#define startskipping()                                       \
  startwrite = -1
#endif
#define bufferflush() startskipping()

  while(count--) {
    c = inbuf[in];

    switch (tn->telrcv_state) {
    case CURL_TS_CR:
      tn->telrcv_state = CURL_TS_DATA;
      if(c == '\0') {
        startskipping();
        break;   /* Ignore \0 after CR */
      }
      if(startwrite < 0)
        startwrite = in;
      break;

    case CURL_TS_DATA:
      if(c == CURL_IAC) {
        tn->telrcv_state = CURL_TS_IAC;
        startskipping();
        break;
      }
      else if(c == '\r')
        tn->telrcv_state = CURL_TS_CR;

      if(startwrite < 0)
        startwrite = in;
      break;

    case CURL_TS_IAC:
    process_iac:
      //DEBUGASSERT(startwrite < 0);
      switch (c) {
      case CURL_WILL:
        tn->telrcv_state = CURL_TS_WILL;
        break;
      case CURL_WONT:
        tn->telrcv_state = CURL_TS_WONT;
        break;
      case CURL_DO:
        tn->telrcv_state = CURL_TS_DO;
        break;
      case CURL_DONT:
        tn->telrcv_state = CURL_TS_DONT;
        break;
      case CURL_SB:
        CURL_SB_CLEAR(tn);
        tn->telrcv_state = CURL_TS_SB;
        break;
      case CURL_IAC:
        tn->telrcv_state = CURL_TS_DATA;
        if(startwrite < 0)
          startwrite = in;
        break;
      case CURL_DM:
      case CURL_NOP:
      case CURL_GA:
      default:
        tn->telrcv_state = CURL_TS_DATA;
        printoption("RCVD", CURL_IAC, c);
        break;
      }
      break;

      case CURL_TS_WILL:
        printoption("RCVD", CURL_WILL, c);
        tn->please_negotiate = 1;
        rec_will(c);
        tn->telrcv_state = CURL_TS_DATA;
        break;

      case CURL_TS_WONT:
        printoption("RCVD", CURL_WONT, c);
        tn->please_negotiate = 1;
        rec_wont(c);
        tn->telrcv_state = CURL_TS_DATA;
        break;

      case CURL_TS_DO:
        printoption("RCVD", CURL_DO, c);
        tn->please_negotiate = 1;
        rec_do(c);
        tn->telrcv_state = CURL_TS_DATA;
        break;

      case CURL_TS_DONT:
        printoption("RCVD", CURL_DONT, c);
        tn->please_negotiate = 1;
        rec_dont(c);
        tn->telrcv_state = CURL_TS_DATA;
        break;

      case CURL_TS_SB:
        if(c == CURL_IAC)
          tn->telrcv_state = CURL_TS_SE;
        else
          CURL_SB_ACCUM(tn,c);
        break;

      case CURL_TS_SE:
        if(c != CURL_SE) {
          if(c != CURL_IAC) {
            /*
             * This is an error.  We only expect to get "IAC IAC" or "IAC SE".
             * Several things may have happened.  An IAC was not doubled, the
             * IAC SE was left off, or another option got inserted into the
             * suboption are all possibilities.  If we assume that the IAC was
             * not doubled, and really the IAC SE was left off, we could get
             * into an infinate loop here.  So, instead, we terminate the
             * suboption, and process the partial suboption if we can.
             */
            CURL_SB_ACCUM(tn, CURL_IAC);
            CURL_SB_ACCUM(tn, c);
            tn->subpointer -= 2;
            CURL_SB_TERM(tn);

            printoption("In SUBOPTION processing, RCVD", CURL_IAC, c);
            suboption();   /* handle sub-option */
            tn->telrcv_state = CURL_TS_IAC;
            goto process_iac;
          }
          CURL_SB_ACCUM(tn,c);
          tn->telrcv_state = CURL_TS_SB;
        }
        else
        {
          CURL_SB_ACCUM(tn, CURL_IAC);
          CURL_SB_ACCUM(tn, CURL_SE);
          tn->subpointer -= 2;
          CURL_SB_TERM(tn);
          suboption();   /* handle sub-option */
          tn->telrcv_state = CURL_TS_DATA;
        }
        break;
    }
    ++in;
  }
  bufferflush();
  return CURLE_OK;
}
/* Escape and send a telnet data block */
/* TODO: write large chunks of data instead of one byte at a time */
CURLcode telnet::send_telnet_data(char *buffer, ssize_t nread)
{
  unsigned char outbuf[2];
  ssize_t bytes_written, total_written;
  int out_count;
  CURLcode rc = CURLE_OK;

  while(rc == CURLE_OK && nread--) {
    outbuf[0] = *buffer++;
    out_count = 1;
    if(outbuf[0] == CURL_IAC)
      outbuf[out_count++] = CURL_IAC;

    total_written = 0;
    do {
      /* Make sure socket is writable to avoid EWOULDBLOCK condition */
      struct pollfd pfd[1];
      pfd[0].fd = sockfd;
      pfd[0].events = POLLOUT;
      switch (Curl_poll(pfd, 1, -1)) {
        case -1:                    /* error, abort writing */
        case 0:                     /* timeout (will never happen) */
          rc = CURLE_SEND_ERROR;
          break;
        default:                    /* write! */
          bytes_written = 0;
          rc = Curl_write(sockfd, outbuf+total_written,
                          out_count-total_written, &bytes_written);
          total_written += bytes_written;
          break;
      }
    /* handle partial write */
    } while(rc == CURLE_OK && total_written < out_count);
  }
  return rc;
}

telnet::telnet(curl_socket_t sock) {
  memset(us, CURL_NO, sizeof us);
  memset(usq, CURL_NO, sizeof usq);
  memset(us_preferred, CURL_NO, sizeof us_preferred);
  memset(him, CURL_NO, sizeof him);
  memset(himq, CURL_NO, sizeof himq);
  memset(him_preferred, CURL_NO, sizeof him_preferred);
  memset(subopt_ttype, 0, sizeof subopt_ttype);
  memset(subopt_xdisploc, 0, sizeof subopt_xdisploc);
  memset(subbuffer, 0, sizeof subbuffer);

  //struct SessionHandle *data = conn->data;
  //curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  sockfd = sock;
  verbose = true;

  telrcv_state = CURL_TS_DATA;

  /* Init suboptions */
  //CURL_SB_CLEAR(this);
  subpointer = subbuffer;
  subend = NULL;

  /* Set the options we want by default */
  us_preferred[CURL_TELOPT_BINARY] = CURL_YES;
  us_preferred[CURL_TELOPT_SGA] = CURL_YES;
  him_preferred[CURL_TELOPT_BINARY] = CURL_YES;
  him_preferred[CURL_TELOPT_SGA] = CURL_YES;

  already_negotiated = 0;
  please_negotiate = 0;
  telnet_vars = NULL;
}

int telnet::setup() {
 CURLcode code;
  ssize_t nread;
  struct timeval now;
  bool keepon = TRUE;
  char buf[BUFSIZE] = {0};

while(keepon) {
    /* read data from network */
    code = Curl_read(sockfd, buf, BUFSIZE - 1, &nread);
    LOGD("read %d bytes, '%s'", nread, buf);
    /* read would've blocked. Loop again */
    if(code == CURLE_AGAIN)
      continue;//break;
    /* returned not-zero, this an error */
    else if(code) {
      //keepon = FALSE;
      break;
    }
    /* returned zero but actually received 0 or less here,
       the server closed the connection and we bail out */
    else if(nread <= 0) {
      //keepon = FALSE;
      break;
    }

    code = telrcv((unsigned char *)buf, nread);
    if(code) {
      //keepon = FALSE;
      break;
    }

    /* Negotiate if the peer has started negotiating,
       otherwise don't. We don't want to speak telnet with
       non-telnet servers, like POP or SMTP. */
    if(please_negotiate && !already_negotiated) {
      negotiate();
      already_negotiated = 1;
    }
    keepon = FALSE;
    }
    return code;
}


#if 0
  HMODULE wsock2;
  WSOCK2_FUNC close_event_func;
  WSOCK2_FUNC create_event_func;
  WSOCK2_FUNC event_select_func;
  WSOCK2_FUNC enum_netevents_func;
  WSAEVENT event_handle;
  WSANETWORKEVENTS events;
  HANDLE stdin_handle;
  HANDLE objs[2];
  DWORD  obj_count;
  DWORD  wait_timeout;
  DWORD waitret;
  DWORD readfile_read;
  int err;

  ssize_t nread;
  struct timeval now;
  bool keepon = TRUE;
  char *buf = data->state.buffer;
  //struct TELNET *tn;
  telnet* tn = this;

  *done = TRUE; /* unconditionally */

  //code = init_telnet(conn);
  //if(code)
  //  return code;

  //tn = (struct TELNET *)data->state.proto.telnet;

  code = check_telnet_options();

  /*
  ** This functionality only works with WinSock >= 2.0.  So,
  ** make sure have it.
  */
  code = check_wsock2();
  if(code)
    return code;

  /* OK, so we have WinSock 2.0.  We need to dynamically */
  /* load ws2_32.dll and get the function pointers we need. */
  wsock2 = LoadLibrary("WS2_32.DLL");
  if(wsock2 == NULL) {
    LOGD("failed to load WS2_32.DLL (%d)", ERRNO);
    return CURLE_FAILED_INIT;
  }

  /* Grab a pointer to WSACreateEvent */
  create_event_func = GetProcAddress(wsock2,"WSACreateEvent");
  if(create_event_func == NULL) {
    LOGD("failed to find WSACreateEvent function (%d)",
          ERRNO);
    FreeLibrary(wsock2);
    return CURLE_FAILED_INIT;
  }

  /* And WSACloseEvent */
  close_event_func = GetProcAddress(wsock2,"WSACloseEvent");
  if(close_event_func == NULL) {
    LOGD("failed to find WSACloseEvent function (%d)",
          ERRNO);
    FreeLibrary(wsock2);
    return CURLE_FAILED_INIT;
  }

  /* And WSAEventSelect */
  event_select_func = GetProcAddress(wsock2,"WSAEventSelect");
  if(event_select_func == NULL) {
    LOGD("failed to find WSAEventSelect function (%d)",
          ERRNO);
    FreeLibrary(wsock2);
    return CURLE_FAILED_INIT;
  }

  /* And WSAEnumNetworkEvents */
  enum_netevents_func = GetProcAddress(wsock2,"WSAEnumNetworkEvents");
  if(enum_netevents_func == NULL) {
    LOGD("failed to find WSAEnumNetworkEvents function (%d)",
          ERRNO);
    FreeLibrary(wsock2);
    return CURLE_FAILED_INIT;
  }

  /* We want to wait for both stdin and the socket. Since
  ** the select() function in winsock only works on sockets
  ** we have to use the WaitForMultipleObjects() call.
  */

  /* First, create a sockets event object */
  event_handle = (WSAEVENT)create_event_func();
  if(event_handle == WSA_INVALID_EVENT) {
    LOGD("WSACreateEvent failed (%d)", SOCKERRNO);
    FreeLibrary(wsock2);
    return CURLE_FAILED_INIT;
  }

  /* Tell winsock what events we want to listen to */
  if(event_select_func(sockfd, event_handle, FD_READ|FD_CLOSE) ==
     SOCKET_ERROR) {
    close_event_func(event_handle);
    FreeLibrary(wsock2);
    return CURLE_OK;
  }

  /* The get the Windows file handle for stdin */
  stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

  /* Create the list of objects to wait for */
  objs[0] = event_handle;
  objs[1] = stdin_handle;

  /* If stdin_handle is a pipe, use PeekNamedPipe() method to check it,
     else use the old WaitForMultipleObjects() way */
  if(GetFileType(stdin_handle) == FILE_TYPE_PIPE ||
     data->set.is_fread_set) {
    /* Don't wait for stdin_handle, just wait for event_handle */
    obj_count = 1;
    /* Check stdin_handle per 100 milliseconds */
    wait_timeout = 100;
  }
  else {
    obj_count = 2;
    wait_timeout = 1000;
  }

  /* Keep on listening and act on events */
  while(keepon) {
    waitret = WaitForMultipleObjects(obj_count, objs, FALSE, wait_timeout);
    switch(waitret) {
    case WAIT_TIMEOUT:
    {
      for(;;) {
        if(obj_count == 1) {
          /* read from user-supplied method */
          code = (int)conn->fread_func(buf, 1, BUFSIZE - 1, conn->fread_in);
          if(code == CURL_READFUNC_ABORT) {
            keepon = FALSE;
            code = CURLE_READ_ERROR;
            break;
          }

          if(code == CURL_READFUNC_PAUSE)
            break;

          if(code == 0)                        /* no bytes */
            break;

          readfile_read = code; /* fall thru with number of bytes read */
        }
        else {
          /* read from stdin */
          if(!PeekNamedPipe(stdin_handle, NULL, 0, NULL,
                            &readfile_read, NULL)) {
            keepon = FALSE;
            code = CURLE_READ_ERROR;
            break;
          }

          if(!readfile_read)
            break;

          if(!ReadFile(stdin_handle, buf, sizeof(data->state.buffer),
                       &readfile_read, NULL)) {
            keepon = FALSE;
            code = CURLE_READ_ERROR;
            break;
          }
        }

        code = send_telnet_data(conn, buf, readfile_read);
        if(code) {
          keepon = FALSE;
          break;
        }
      }
    }
    break;

    case WAIT_OBJECT_0 + 1:
    {
      if(!ReadFile(stdin_handle, buf, sizeof(data->state.buffer),
                   &readfile_read, NULL)) {
        keepon = FALSE;
        code = CURLE_READ_ERROR;
        break;
      }

      code = send_telnet_data(conn, buf, readfile_read);
      if(code) {
        keepon = FALSE;
        break;
      }
    }
    break;

    case WAIT_OBJECT_0:

      if(SOCKET_ERROR == enum_netevents_func(sockfd, event_handle, &events)) {
        if((err = SOCKERRNO) != EINPROGRESS) {
          LOGE("WSAEnumNetworkEvents failed (%d)", err);
          keepon = FALSE;
          code = CURLE_READ_ERROR;
        }
        break;
      }
      if(events.lNetworkEvents & FD_READ) {
        /* read data from network */
        code = Curl_read(sockfd, buf, BUFSIZE - 1, &nread);
        /* read would've blocked. Loop again */
        if(code == CURLE_AGAIN)
          break;
        /* returned not-zero, this an error */
        else if(code) {
          keepon = FALSE;
          break;
        }
        /* returned zero but actually received 0 or less here,
           the server closed the connection and we bail out */
        else if(nread <= 0) {
          keepon = FALSE;
          break;
        }

        code = telrcv((unsigned char *)buf, nread);
        if(code) {
          keepon = FALSE;
          break;
        }

        /* Negotiate if the peer has started negotiating,
           otherwise don't. We don't want to speak telnet with
           non-telnet servers, like POP or SMTP. */
        if(tn->please_negotiate && !tn->already_negotiated) {
          negotiate();
          tn->already_negotiated = 1;
        }
      }
      if(events.lNetworkEvents & FD_CLOSE) {
        keepon = FALSE;
      }
      break;

    }

    if(data->set.timeout) {
      now = Curl_tvnow();
      if(Curl_tvdiff(now, conn->created) >= data->set.timeout) {
        LOGD( "Time-out");
        code = CURLE_OPERATION_TIMEDOUT;
        keepon = FALSE;
      }
    }
  }

  /* We called WSACreateEvent, so call WSACloseEvent */
  if(!close_event_func(event_handle)) {
    LOGE("WSACloseEvent failed (%d)", SOCKERRNO);
  }

  /* "Forget" pointers into the library we're about to free */
  create_event_func = NULL;
  close_event_func = NULL;
  event_select_func = NULL;
  enum_netevents_func = NULL;

  /* We called LoadLibrary, so call FreeLibrary */
  if(!FreeLibrary(wsock2))
    LOGE("FreeLibrary(wsock2) failed (%d)", ERRNO);

  /* mark this as "no further transfer wanted" */
  Curl_setup_transfer(conn, -1, -1, FALSE, NULL, -1, NULL);

  return code;
}

static CURLcode check_wsock2 () {
  int err;
  WORD wVersionRequested;
  WSADATA wsaData;

 // DEBUGASSERT(data);

  /* telnet requires at least WinSock 2.0 so ask for it. */
  wVersionRequested = MAKEWORD(2, 0);

  err = WSAStartup(wVersionRequested, &wsaData);

  /* We must've called this once already, so this call */
  /* should always succeed.  But, just in case... */
  if(err != 0) {
    LOGE("WSAStartup failed (%d)",err);
    return CURLE_FAILED_INIT;
  }

  /* We have to have a WSACleanup call for every successful */
  /* WSAStartup call. */
  WSACleanup();

  /* Check that our version is supported */
  if(LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
      HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)) {
      /* Our version isn't supported */
      LOGE("insufficient winsock version to support "
            "telnet");
      return CURLE_FAILED_INIT;
  }

  /* Our version is supported */
  return CURLE_OK;
#endif

/*
 * This is a wrapper around poll().  If poll() does not exist, then
 * select() is used instead.  An error is returned if select() is
 * being used and a file descriptor is too large for FD_SETSIZE.
 * A negative timeout value makes this function wait indefinitely,
 * unles no valid file descriptor is given, when this happens the
 * negative timeout is ignored and the function times out immediately.
 * When compiled with CURL_ACKNOWLEDGE_EINTR defined, EINTR condition
 * is honored and function might exit early without awaiting timeout,
 * otherwise EINTR will be ignored.
 *
 * Return values:
 *   -1 = system call error or fd >= FD_SETSIZE
 *    0 = timeout
 *    N = number of structures with non zero revent fields
 */
int Curl_poll(struct pollfd ufds[], unsigned int nfds, int timeout_ms)
{
#ifndef HAVE_POLL_FINE
  struct timeval pending_tv;
  struct timeval *ptimeout;
  fd_set fds_read;
  fd_set fds_write;
  fd_set fds_err;
  curl_socket_t maxfd;
#endif
  struct timeval initial_tv = {0,0};
  bool fds_none = TRUE;
  unsigned int i;
  int pending_ms = 0;
  int error;
  int r;

  if(ufds) {
    for(i = 0; i < nfds; i++) {
      if(ufds[i].fd != CURL_SOCKET_BAD) {
        fds_none = FALSE;
        break;
      }
    }
  }
  if(fds_none) {
    r = Curl_wait_ms(timeout_ms);
    return r;
  }

  /* Avoid initial timestamp, avoid curlx_tvnow() call, when elapsed
     time in this function does not need to be measured. This happens
     when function is called with a zero timeout or a negative timeout
     value indicating a blocking call should be performed. */

  if(timeout_ms > 0) {
    pending_ms = timeout_ms;
    initial_tv = curlx_tvnow();
  }

#ifdef HAVE_POLL_FINE

  do {
    if(timeout_ms < 0)
      pending_ms = -1;
    else if(!timeout_ms)
      pending_ms = 0;
    r = poll(ufds, nfds, pending_ms);
    if(r != -1)
      break;
    error = SOCKERRNO;
    if(error && error_not_EINTR)
      break;
    if(timeout_ms > 0) {
      pending_ms = timeout_ms - elapsed_ms;
      if(pending_ms <= 0)
        break;
    }
  } while(r == -1);

  if(r < 0)
    return -1;
  if(r == 0)
    return 0;

  for(i = 0; i < nfds; i++) {
    if(ufds[i].fd == CURL_SOCKET_BAD)
      continue;
    if(ufds[i].revents & POLLHUP)
      ufds[i].revents |= POLLIN;
    if(ufds[i].revents & POLLERR)
      ufds[i].revents |= (POLLIN|POLLOUT);
  }

#else  /* HAVE_POLL_FINE */

  FD_ZERO(&fds_read);
  FD_ZERO(&fds_write);
  FD_ZERO(&fds_err);
  maxfd = (curl_socket_t)-1;

  for(i = 0; i < nfds; i++) {
    ufds[i].revents = 0;
    if(ufds[i].fd == CURL_SOCKET_BAD)
      continue;
    VERIFY_SOCK(ufds[i].fd);
    if(ufds[i].events & (POLLIN|POLLOUT|POLLPRI|
                          POLLRDNORM|POLLWRNORM|POLLRDBAND)) {
      if(ufds[i].fd > maxfd)
        maxfd = ufds[i].fd;
      if(ufds[i].events & (POLLRDNORM|POLLIN))
        FD_SET(ufds[i].fd, &fds_read);
      if(ufds[i].events & (POLLWRNORM|POLLOUT))
        FD_SET(ufds[i].fd, &fds_write);
      if(ufds[i].events & (POLLRDBAND|POLLPRI))
        FD_SET(ufds[i].fd, &fds_err);
    }
  }

  ptimeout = (timeout_ms < 0) ? NULL : &pending_tv;

  do {
    if(timeout_ms > 0) {
      pending_tv.tv_sec = pending_ms / 1000;
      pending_tv.tv_usec = (pending_ms % 1000) * 1000;
    }
    else if(!timeout_ms) {
      pending_tv.tv_sec = 0;
      pending_tv.tv_usec = 0;
    }
    r = select((int)maxfd + 1, &fds_read, &fds_write, &fds_err, ptimeout);
    if(r != -1)
      break;
    error = SOCKERRNO;
    if(error && error_not_EINTR)
      break;
    if(timeout_ms > 0) {
      pending_ms = timeout_ms - elapsed_ms;
      if(pending_ms <= 0)
        break;
    }
  } while(r == -1);

  if(r < 0)
    return -1;
  if(r == 0)
    return 0;

  r = 0;
  for(i = 0; i < nfds; i++) {
    ufds[i].revents = 0;
    if(ufds[i].fd == CURL_SOCKET_BAD)
      continue;
    if(FD_ISSET(ufds[i].fd, &fds_read))
      ufds[i].revents |= POLLIN;
    if(FD_ISSET(ufds[i].fd, &fds_write))
      ufds[i].revents |= POLLOUT;
    if(FD_ISSET(ufds[i].fd, &fds_err))
      ufds[i].revents |= POLLPRI;
    if(ufds[i].revents != 0)
      r++;
  }

#endif  /* HAVE_POLL_FINE */

  return r;
}

CURLcode Curl_write(curl_socket_t sockfd,
                    const void *mem,
                    size_t len,
                    ssize_t *written)
{
  ssize_t bytes_written;
  CURLcode curlcode = CURLE_OK;
//  int num = (sockfd == conn->sock[SECONDARYSOCKET]);
//  bytes_written = conn->send[num](conn, num, mem, len, &curlcode);
//  curl_socket_t sockfd = conn->sock[num];
  bytes_written = swrite(sockfd, mem, len);

  if(-1 == bytes_written) {
    int err = SOCKERRNO;

    if(/* This is how Windows does it */
      (WSAEWOULDBLOCK == err)) {
      /* this is just a case of EWOULDBLOCK */
      bytes_written=0;
      curlcode = CURLE_AGAIN;
    }
    else {
      LOGE( "Send failure: %s", strerror(err));
      curlcode = CURLE_SEND_ERROR;
    }
  }


  *written = bytes_written;
  if(bytes_written >= 0)
    /* we completely ignore the curlcode value when subzero is not returned */
    return CURLE_OK;

  /* handle CURLE_AGAIN or a send failure */
  switch(curlcode) {
  case CURLE_AGAIN:
    *written = 0;
    return CURLE_OK;

  case CURLE_OK:
    /* general send failure */
    return CURLE_SEND_ERROR;

  default:
    /* we got a specific curlcode, forward it */
    return (CURLcode)curlcode;
  }
}

/*
 * Internal read-from-socket function. This is meant to deal with plain
 * sockets, SSL sockets and kerberos sockets.
 *
 * Returns a regular CURLcode value.
 */
CURLcode Curl_read(/* connection data */
                   curl_socket_t sockfd,     /* read from this socket */
                   char *buf,                /* store read data here */
                   size_t sizerequested,     /* max amount to read */
                   ssize_t *n)               /* amount bytes read */
{
  CURLcode curlcode = CURLE_RECV_ERROR;
  ssize_t nread = 0;

  /* Set 'num' to 0 or 1, depending on which socket that has been sent here.
     If it is the second socket, we set num to 1. Otherwise to 0. This lets
     us use the correct ssl handle. */
  //int num = (sockfd == conn->sock[SECONDARYSOCKET]);

  *n=0; /* reset amount to zero */

  //nread = conn->recv[num](conn, num, buffertofill, bytesfromsocket, &curlcode);
  //curl_socket_t sockfd = conn->sock[num];
  nread = sread(sockfd, buf, sizerequested);

  curlcode = CURLE_OK;
  if(-1 == nread) {
    int err = SOCKERRNO;

      /* This is how Windows does it */
    if((WSAEWOULDBLOCK == err)) {
      /* this is just a case of EWOULDBLOCK */
      curlcode = CURLE_AGAIN;
    } else {
      LOGD("Recv failure: %s", strerror(err));
      curlcode = CURLE_RECV_ERROR;
    }
  }

  if(nread < 0)
    return curlcode;

  *n += nread;

  return CURLE_OK;
}

/* Portable, consistent toupper (remember EBCDIC). Do not use toupper() because
   its behavior is altered by the current locale. */
char Curl_raw_toupper(char in)
{
  switch (in) {
  case 'a':
    return 'A';
  case 'b':
    return 'B';
  case 'c':
    return 'C';
  case 'd':
    return 'D';
  case 'e':
    return 'E';
  case 'f':
    return 'F';
  case 'g':
    return 'G';
  case 'h':
    return 'H';
  case 'i':
    return 'I';
  case 'j':
    return 'J';
  case 'k':
    return 'K';
  case 'l':
    return 'L';
  case 'm':
    return 'M';
  case 'n':
    return 'N';
  case 'o':
    return 'O';
  case 'p':
    return 'P';
  case 'q':
    return 'Q';
  case 'r':
    return 'R';
  case 's':
    return 'S';
  case 't':
    return 'T';
  case 'u':
    return 'U';
  case 'v':
    return 'V';
  case 'w':
    return 'W';
  case 'x':
    return 'X';
  case 'y':
    return 'Y';
  case 'z':
    return 'Z';
  }
  return in;
}

/*
 * Curl_raw_equal() is for doing "raw" case insensitive strings. This is meant
 * to be locale independent and only compare strings we know are safe for
 * this.  See http://daniel.haxx.se/blog/2008/10/15/strcasecmp-in-turkish/ for
 * some further explanation to why this function is necessary.
 *
 * The function is capable of comparing a-z case insensitively even for
 * non-ascii.
 */

int Curl_raw_equal(const char *first, const char *second)
{
  while(*first && *second) {
    if(Curl_raw_toupper(*first) != Curl_raw_toupper(*second))
      /* get out of the loop as soon as they don't match */
      break;
    first++;
    second++;
  }
  /* we do the comparison here (possibly again), just to make sure that if the
     loop above is skipped because one of the strings reached zero, we must not
     return this as a successful match */
  return (Curl_raw_toupper(*first) == Curl_raw_toupper(*second));
}

/* returns last node in linked list */
static struct curl_slist *slist_get_last(struct curl_slist *list)
{
  struct curl_slist     *item;

  /* if caller passed us a NULL, return now */
  if(!list)
    return NULL;

  /* loop through to find the last item */
  item = list;
  while(item->next) {
    item = item->next;
  }
  return item;
}

/*
 * curl_slist_append() appends a string to the linked list. It always returns
 * the address of the first record, so that you can use this function as an
 * initialization function as well as an append function. If you find this
 * bothersome, then simply create a separate _init function and call it
 * appropriately from within the program.
 */
struct curl_slist *curl_slist_append(struct curl_slist *list,
                                     const char *data)
{
  struct curl_slist     *last;
  struct curl_slist     *new_item;

  new_item = (struct curl_slist*)malloc(sizeof(struct curl_slist));
  if(new_item) {
    char *dupdata = strdup(data);
    if(dupdata) {
      new_item->next = NULL;
      new_item->data = dupdata;
    }
    else {
      free(new_item);
      return NULL;
    }
  }
  else
    return NULL;

  if(list) {
    last = slist_get_last(list);
    last->next = new_item;
    return list;
  }

  /* if this is the first item, then new_item *is* the list */
  return new_item;
}

/*
 * Curl_slist_duplicate() duplicates a linked list. It always returns the
 * address of the first record of the cloned list or NULL in case of an
 * error (or if the input list was NULL).
 */
struct curl_slist *Curl_slist_duplicate(struct curl_slist *inlist)
{
  struct curl_slist *outlist = NULL;
  struct curl_slist *tmp;

  while(inlist) {
    tmp = curl_slist_append(outlist, inlist->data);

    if(!tmp) {
      curl_slist_free_all(outlist);
      return NULL;
    }

    outlist = tmp;
    inlist = inlist->next;
  }
  return outlist;
}

/* be nice and clean up resources */
void curl_slist_free_all(struct curl_slist *list)
{
  struct curl_slist     *next;
  struct curl_slist     *item;

  if(!list)
    return;

  item = list;
  do {
    next = item->next;
    Curl_safefree(item->data);
    free(item);
    item = next;
  } while(next);
}

#if 0
/*
 * Our thread-safe and smart strerror() replacement.
 *
 * The 'err' argument passed in to this function MUST be a true errno number
 * as reported on this system. We do no range checking on the number before
 * we pass it to the "number-to-message" conversion function and there might
 * be systems that don't do proper range checking in there themselves.
 *
 * We don't do range checking (on systems other than Windows) since there is
 * no good reliable and portable way to do it.
 */
const char *Curl_strerror( int err)
{
  char *buf, *p;
  size_t max;
  int old_errno = ERRNO;

  DEBUGASSERT(conn);
  DEBUGASSERT(err >= 0);

  buf = conn->syserr_buf;
  max = sizeof(conn->syserr_buf)-1;
  *buf = '\0';

#ifdef USE_WINSOCK
  /* 'sys_nerr' is the maximum errno number, it is not widely portable */
  if(err >= 0 && err < sys_nerr)
    strncpy(buf, strerror(err), max);
  else {
    if(!get_winsock_error(err, buf, max) &&
        !FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                       LANG_NEUTRAL, buf, (DWORD)max, NULL))
      snprintf(buf, max, "Unknown error %d (%#x)", err, err);
  }

#else /* not USE_WINSOCK coming up */
  {
    char *msg = strerror(err);
    if(msg)
      strncpy(buf, msg, max);
    else
      snprintf(buf, max, "Unknown error %d", err);
  }

#endif /* end of ! USE_WINSOCK */

  buf[max] = '\0'; /* make sure the string is zero terminated */

  /* strip trailing '\r\n' or '\n'. */
  if((p = strrchr(buf,'\n')) != NULL && (p - buf) >= 2)
     *p = '\0';
  if((p = strrchr(buf,'\r')) != NULL && (p - buf) >= 1)
     *p = '\0';

  if(old_errno != ERRNO)
    SET_ERRNO(old_errno);

  return buf;
}
#endif

struct timeval curlx_tvnow(void)
{
  /*
  ** GetTickCount() is available on _all_ Windows versions from W95 up
  ** to nowadays. Returns milliseconds elapsed since last system boot,
  ** increases monotonically and wraps once 49.7 days have elapsed.
  */
  struct timeval now;
  DWORD milliseconds = GetTickCount();
  now.tv_sec = milliseconds / 1000;
  now.tv_usec = (milliseconds % 1000) * 1000;
  return now;
}

/*
 * Make sure that the first argument is the more recent time, as otherwise
 * we'll get a weird negative time-diff back...
 *
 * Returns: the time difference in number of milliseconds.
 */
long curlx_tvdiff(struct timeval newer, struct timeval older)
{
  return (newer.tv_sec-older.tv_sec)*1000+
    (newer.tv_usec-older.tv_usec)/1000;
}


int Curl_wait_ms(int timeout_ms)
{
#if !defined(MSDOS) && !defined(USE_WINSOCK)
#ifndef HAVE_POLL_FINE
  struct timeval pending_tv;
#endif
  struct timeval initial_tv;
  int pending_ms;
  int error;
#endif
  int r = 0;

  if(!timeout_ms)
    return 0;
  if(timeout_ms < 0) {
    SET_SOCKERRNO(EINVAL);
    return -1;
  }
#if defined(MSDOS)
  delay(timeout_ms);
#elif defined(USE_WINSOCK)
  Sleep(timeout_ms);
#else
  pending_ms = timeout_ms;
  initial_tv = curlx_tvnow();
  do {
#if defined(HAVE_POLL_FINE)
    r = poll(NULL, 0, pending_ms);
#else
    pending_tv.tv_sec = pending_ms / 1000;
    pending_tv.tv_usec = (pending_ms % 1000) * 1000;
    r = select(0, NULL, NULL, NULL, &pending_tv);
#endif /* HAVE_POLL_FINE */
    if(r != -1)
      break;
    error = SOCKERRNO;
    if(error && error_not_EINTR)
      break;
    pending_ms = timeout_ms - elapsed_ms;
    if(pending_ms <= 0)
      break;
  } while(r == -1);
#endif /* USE_WINSOCK */
  if(r)
    r = -1;
  return r;
}
