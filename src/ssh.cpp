/*!
ssh.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include"ssh.h"


static int waitsocket(int socket_fd, LIBSSH2_SESSION* session){
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set* writefd = NULL;
    fd_set* readfd = NULL;
    int dir;

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    FD_ZERO(&fd);

    FD_SET(socket_fd, &fd);

    /* now make sure we wait in the correct direction */
    dir = libssh2_session_block_directions(session);

    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

SSH::SSH(){
    WSADATA wsadata;
    int err;
    int rc;
    char errtxt[1024];

#ifdef WIN32
    err = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if(err != 0){
        snprintf(errtxt, 1024,"WSAStartup failed with error: %d\n", err);
        throw errtxt;
    }
#endif
    
    rc = libssh2_init(0);
    if(rc != 0){
        snprintf(errtxt, 1024, "libssh2 initialization failed (%d)\n", rc);
        throw errtxt;
    }
    connect_flag = false;
}

SSH::~SSH(){
#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    libssh2_exit();

}

int SSH::Connect(char* hostname, char* username, char* password){
    unsigned long hostaddr;
    int rc;
    struct sockaddr_in sin;
    const char* fingerprint;
    size_t len;
    LIBSSH2_KNOWNHOSTS* nh;
    int type;
    char ip[32];

    LPHOSTENT host = gethostbyname(hostname);
    if(host == NULL){
        return -1;
    }
    snprintf(ip,32, "%d.%d.%d.%d", (BYTE) * ((host->h_addr_list[0])),
        (BYTE) * ((host->h_addr_list[0]) + 1),
        (BYTE) * ((host->h_addr_list[0]) + 2),
        (BYTE) * ((host->h_addr_list[0]) + 3));
    hostaddr = inet_addr(ip);

    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    if(connect(sock, (struct sockaddr*)(&sin),
        sizeof(struct sockaddr_in)) != 0){
        fprintf(stderr, "failed to connect!\n");
        return -1;
    }
    /* Create a session instance */
    session = libssh2_session_init();
    if(!session)
        return -1;

    /* tell libssh2 we want it all done non-blocking */
    libssh2_session_set_blocking(session, 0);

    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    while((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN);
    if(rc){
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        return -1;
    }
    nh = libssh2_knownhost_init(session);
    if(!nh){
        /* eeek, do cleanup here */
        return 2;
    }
    /* read all hosts from here */
    libssh2_knownhost_readfile(nh, "known_hosts",
        LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    /* store all known hosts to here */
    libssh2_knownhost_writefile(nh, "dumpfile",
        LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    fingerprint = libssh2_session_hostkey(session, &len, &type);
    if(fingerprint){
        struct libssh2_knownhost* host;
#if LIBSSH2_VERSION_NUM >= 0x010206
        /* introduced in 1.2.6 */
        int check = libssh2_knownhost_checkp(nh, hostname, 22,
            fingerprint, len,
            LIBSSH2_KNOWNHOST_TYPE_PLAIN |
            LIBSSH2_KNOWNHOST_KEYENC_RAW,
            &host);
#else
        /* 1.2.5 or older */
        int check = libssh2_knownhost_check(nh, hostname,
            fingerprint, len,
            LIBSSH2_KNOWNHOST_TYPE_PLAIN |
            LIBSSH2_KNOWNHOST_KEYENC_RAW,
            &host);
#endif
        /*fprintf(stderr, "Host check: %d, key: %s\n", check,
            (check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH) ?
            host->key : "<none>");*/

        /*****
         * At this point, we could verify that 'check' tells us the key is
         * fine or bail out.
         *****/
    } else{
        /* eeek, do cleanup here */
        return 3;
    }
    libssh2_knownhost_free(nh);

    if(strlen(password) != 0){
        /* We could authenticate via password */
        while((rc = libssh2_userauth_password(session, username, password)) == LIBSSH2_ERROR_EAGAIN);
        if(rc){
            fprintf(stderr, "Authentication by password failed.\n");
            Shutdown();
            return -1;
        }
    } else{
        /* Or by public key */
        while((rc = libssh2_userauth_publickey_fromfile(session, username,
            "/home/user/"
            ".ssh/id_rsa.pub",
            "/home/user/"
            ".ssh/id_rsa",
            password)) ==
            LIBSSH2_ERROR_EAGAIN);
        if(rc){
            fprintf(stderr, "\tAuthentication by public key failed\n");
            Shutdown();
            return -1;
        }
    }
    
    connect_flag = true;
    return 0;
}

int SSH::ExecCmd(char* cmd,char*buffer){
    int rc;
    int bytecount = 0;
    int exit_code;
    int wait_cnt = 0;
    /* Exec non-blocking on the remove host */
    while((channel = libssh2_channel_open_session(session)) == NULL &&
        libssh2_session_last_error(session, NULL, NULL, 0) ==
        LIBSSH2_ERROR_EAGAIN){
        waitsocket(sock, session);
        if(wait_cnt > 20){
            return -2;
        }
        wait_cnt++;
    }
    if(channel == NULL){
        fprintf(stderr, "Error\n");
        return -1;
    }
    wait_cnt = 0;
    while((rc = libssh2_channel_exec(channel, cmd)) ==
        LIBSSH2_ERROR_EAGAIN){
        waitsocket(sock, session);
        if(wait_cnt > 20){
            return -2;
        }
        wait_cnt++;
    }
    if(rc != 0){
        fprintf(stderr, "Error\n");
        return -1;
    }
    wait_cnt = 0;
    for(;;){
        /* loop until we block */
        int rc;
        do{
            rc = libssh2_channel_read(channel, buffer, sizeof(buffer));
            if(rc > 0){
                buffer += rc;
                //*(buffer++) = '\n';
                wait_cnt = 0;
            }else{
                if(rc != LIBSSH2_ERROR_EAGAIN){
                    /* no need to output this for the EAGAIN case */
                    exit_code = rc;
                }
            }
        } while(rc > 0);

        /* this is due to blocking that would occur otherwise so we loop on
            this condition */
        if(rc == LIBSSH2_ERROR_EAGAIN){
            waitsocket(sock, session);
            if(wait_cnt > 20){
                return -2;
            }
            wait_cnt++;
        } else
            break;
    }
    while((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN)
        waitsocket(sock, session);

    int exitcode = 127;
    char* exitsignal = (char*)"none";
    if(rc == 0){
        exitcode = libssh2_channel_get_exit_status(channel);
        libssh2_channel_get_exit_signal(channel, &exitsignal,
            NULL, NULL, NULL, NULL, NULL);
    }

    libssh2_channel_free(channel);
    channel = NULL;
    return exit_code;
    
}

void SSH::DisConnect(){
    libssh2_session_disconnect(session,
        "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);
    connect_flag = false;
}

void SSH::Shutdown(){
    if(connect_flag){
        DisConnect();
    }
    closesocket(sock);
    libssh2_exit();
}

