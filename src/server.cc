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
static int delay_time=0;
static std::string bind_ip;
static int bind_port=0;
static bool echo_char=false;
static bool reuseaddr=false;
static void usage(){
    fprintf(stderr,"usage:%s [-r] [-d delay] [-b ip] [-c] port\n",prog);
}
void sig_handler(int signo){
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
                case 'r':
                    reuseaddr=true;
                    break;
                case 'b':
                    bind_ip=argv[i+1];
                    i++;
                    break;
                case 'd':
                    delay_time=atoi(argv[i+1]);
                    i++;
                    break;
                case 'c':
                    echo_char=true;
                    break;
                default:
                    fprintf(stderr,"%s:unknown option '%s'\n",prog,argv[i]);
                    usage();
                    return -1;
            }
        }else{
            bind_port=atoi(argv[i]);
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
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(bind_port);
    if(bind_ip.size()){
        inet_pton(AF_INET,bind_ip.c_str(),&(addr.sin_addr));
        printf("bind (%s:%d)\n",bind_ip.c_str(),bind_port);
    }else{
        addr.sin_addr.s_addr=INADDR_ANY;
        printf("bind (0.0.0.0:%d)\n",bind_port);
    }
    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
        printf("bind failed:%m\n");
        return -1;
    }
    printf("listen 5\n");
    if(listen(fd,5)==-1){
        printf("listen failed:%m\n");
        return -1;
    }
    if(delay_time){
        sleep(delay_time);
    }
    signal(SIGPIPE,sig_handler);
    while(1){
        sockaddr_in cli_addr;
        socklen_t cli_addr_size=sizeof(cli_addr);
        printf("accept ...\n");
        int conn_fd=accept(fd,(struct sockaddr*)&cli_addr,&cli_addr_size);
        if(conn_fd==-1){
            printf("accept failed:%m\n");
            return -1;
        }
        char cli_ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET,&(cli_addr.sin_addr),cli_ip,sizeof(cli_ip));
        printf("client (%s:%d)\n",cli_ip,ntohs(cli_addr.sin_port));
        if(echo_char){
            char data[1];
            int ret=read(conn_fd,data,sizeof(data));
            if(ret==-1){
                printf("read failed:%m\n");
            }else{
                printf("read succeed=%d\n",ret);
            }
            ret=write(conn_fd,data,sizeof(data));
            if(ret==-1){
                printf("write failed:%m\n");
            }else{
                printf("write succeed=%d\n",ret);
            }
        }
        close(conn_fd);
    }
    return 0;
}

