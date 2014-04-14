#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <signal.h>

static const char* prog=0;
static std::string connect_ip;
static int connect_port=0;
static int bind_port=0;
static bool linger=false;
static bool reuseaddr=false;
static bool half_close=false;

static void usage(){
    fprintf(stderr,"usage:%s [-b port] [-l] [-r] [-H] ip port\n",prog);
}
static void signal_handler(int signo){
    printf("caught signal:%s\n",strsignal(signo));
}

int main(int argc,char* const argv[]){
    prog=argv[0];
    bool except=true;
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'h':
                    usage();
                    return 0;
                case 'b':
                    bind_port=atoi(argv[i+1]);
                    i++;
                    break;
                case 'l':
                    linger=true;
                    break;
                case 'r':
                    reuseaddr=true;
                    break;
                case 'H':
                    half_close=true;
                    break;
                default:
                    fprintf(stderr,"%s:unknown option '%s'\n",prog,argv[i]);
                    usage();
                    return -1;
            }
        }else{
            connect_ip=argv[i];
            connect_port=atoi(argv[i+1]);
            except=false;
            break;
        }
    }
    if(except){
        usage();
        return -1;
    }
    int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(fd==-1){
        printf("socket failed:%m\n");
        return -1;
    }
    if(reuseaddr){
        int ok=1;
        printf("setsockopt SO_REUSEADDR\n");
        if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&ok,sizeof(ok))==-1){
            fprintf(stderr,"setsockopt SO_REUSEADDR failed:%m\n");
            return -1;
        }
    }
    if(linger){
        struct linger val;
        val.l_onoff=1;
        val.l_linger=0;
        printf("setsockopt SO_LINGER\n");
        if(setsockopt(fd,SOL_SOCKET,SO_LINGER,&val,sizeof(val))==-1){
            fprintf(stderr,"setsockopt SO_LINGER failed:%m\n");
            return -1;
        }
    }
    if(bind_port){
        sockaddr_in addr;
        memset(&addr,0,sizeof(addr));
        addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=INADDR_ANY;
        addr.sin_port=htons(bind_port);
        printf("bind (0.0.0.0:%d)\n",bind_port);
        if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
            printf("bind failed:%m\n");
            return -1;
        }
    }
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,connect_ip.c_str(),&(addr.sin_addr));
    addr.sin_port=htons(connect_port);
    printf("connect %s:%d ...\n",connect_ip.c_str(),connect_port);
    if(connect(fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
        printf("connect failed:%m\n");
        return -1;
    }else{
        printf("connect succeed\n");
    }

    signal(SIGPIPE,signal_handler);
    if(half_close){
        char data[1];
        int ret=0;
        //SHUT_RD
        shutdown(fd,SHUT_RD);
        ret=read(fd,data,sizeof(data));
        if(ret==-1){
            printf("read failed:%m\n");
        }else{
            printf("read succeed=%d\n",ret);
        }
        //SHUT_WR
        shutdown(fd,SHUT_WR);
        ret=write(fd,data,sizeof(data));
        if(ret==-1){
            printf("write failed:%m\n");
        }else{
            printf("write succeed=%d\n",ret);
        }
    }
    return 0;
}
