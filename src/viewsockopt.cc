#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <cstdio>

union val{
    int i;
    struct linger l;
    struct timeval t;
};

static const char* sock_str_flag(union val* v){
    static char buf[1024];
    snprintf(buf,sizeof(buf),"%s",v->i?"on":"off");
    return buf;
}
static const char* sock_str_value(union val* v){
    static char buf[1024];
    snprintf(buf,sizeof(buf),"%d",v->i);
    return buf;
}
static const char* sock_str_linger(union val* v){
    static char buf[1024];
    snprintf(buf,sizeof(buf),"l_onoff=%s,l_linger=%d",
             v->l.l_onoff?"on":"off",
             v->l.l_linger);
    return buf;
}
static const char* sock_str_timeval(union val* v){
    static char buf[1024];
    snprintf(buf,sizeof(buf),"(%d,%d)",
             v->t.tv_sec,
             v->t.tv_usec);
    return buf;
}

static struct sock_opt{
    const char* opt_str;
    int opt_level;
    int opt_name;
    const char* (*func)(union val* v);
} sock_opts[]={
    {"SO_ERROR",SOL_SOCKET,SO_ERROR,sock_str_value},
    {"SO_KEEPALIVE",SOL_SOCKET,SO_KEEPALIVE,sock_str_flag},
    {"SO_LINGER",SOL_SOCKET,SO_LINGER,sock_str_linger},
    {"SO_RCVBUF",SOL_SOCKET,SO_RCVBUF,sock_str_value},
    {"SO_SNDBUF",SOL_SOCKET,SO_SNDBUF,sock_str_value},
    {"SO_RCVLOWAT",SOL_SOCKET,SO_RCVLOWAT,sock_str_value},
    {"SO_SNDLOWAT",SOL_SOCKET,SO_SNDLOWAT,sock_str_value},
    {"SO_RCVTIMEO",SOL_SOCKET,SO_RCVTIMEO,sock_str_timeval},
    {"SO_SNDTIMEO",SOL_SOCKET,SO_SNDTIMEO,sock_str_timeval},
    {"SO_REUSEADDR",SOL_SOCKET,SO_REUSEADDR,sock_str_flag},
    {"TCP_MAXSEG",IPPROTO_TCP,TCP_MAXSEG,sock_str_value},
    {"TCP_NODELAY",IPPROTO_TCP,TCP_NODELAY,sock_str_flag},
    {"TCP_QUICKACK",IPPROTO_TCP,TCP_QUICKACK,sock_str_flag},
    {"TCP_CORK",IPPROTO_TCP,TCP_CORK,sock_str_flag},
    {NULL,0,0,NULL}
};

int main(){
    for(int i=0;sock_opts[i].opt_str;i++){
        printf("%s:",sock_opts[i].opt_str);
        int fd=-1;
        switch(sock_opts[i].opt_level){
            case SOL_SOCKET:
            case IPPROTO_TCP:
                fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
                break;
            default:
                continue;
        }
        union val v;
        socklen_t size=sizeof(v);
        getsockopt(fd,
                   sock_opts[i].opt_level,
                   sock_opts[i].opt_name,
                   &v,&size);
        printf("%s\n",sock_opts[i].func(&v));
        close(fd);
    }
    return 0;
}

