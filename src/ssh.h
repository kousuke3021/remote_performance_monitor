#pragma once
#define _WINSOCKAPI_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <libssh2.h>
#include <stdlib.h>
#include <stdio.h> 

#pragma comment(lib,"libssh2.lib")
#pragma comment(lib,"ws2_32.lib")

/*
 * Sample showing how to use libssh2 to execute a command remotely.
 *
 * The sample code has fixed values for host name, user name, password
 * and command to run.
 *
 * Run it like this:
 *
 * $ ./ssh2_exec 127.0.0.1 user password "uptime"
 *
 */

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

class SSH{
    public:
        SSH();
        ~SSH();
        int Connect(char* hostname, char* username, char* password);
        void DisConnect();
        void Shutdown();
        int ExecCmd(char* cmd,char*buffer);

    private:
        char hostname[32] = "";
        char username[128] = "ohashi02";
        char password[128] = "1111";
        LIBSSH2_SESSION* session;
        LIBSSH2_CHANNEL* channel;
        int sock;

    public:
        bool connect_flag;
        bool channel_flag;
};