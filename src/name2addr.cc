#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdio>

static void print_hostent(const char* s,const struct hostent* p){
    printf("====================%s====================\n",s);
    if(p->h_name){
        printf("hostname:%s\n",p->h_name);
    }
    if(p->h_aliases && p->h_aliases[0]){
        printf("alias:\n");
        for(int i=0;p->h_aliases[i];i++){
            printf("\t%s\n",p->h_aliases[i]);
        }
    }
    if(p->h_addr_list && p->h_addr_list[0]){
        printf("ip:\n");
        for(int i=0;p->h_addr_list[i];i++){
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(p->h_addrtype,p->h_addr_list[i],ip,sizeof(ip));
            printf("\t%s\n",ip);
        }
    }
}

int main(int argc,char* const argv[]){
    if(argc!=2){
        fprintf(stderr,"usage:%s hostname\n",argv[0]);
        return -1;
    }
    const char* hostname=argv[1];
    char buf[16*1024];
    struct hostent ent;
    struct hostent* entp;
    int herrno=0;
    {
        //AF_INET
        gethostbyname2_r(hostname,AF_INET,&ent,buf,sizeof(buf),&entp,&herrno);
        if(!entp){
            fprintf(stderr,"gethostbyname2_r(%s,AF_INET) failed:%s\n",
                    hostname,hstrerror(herrno));
            goto next1;
        }else{
            print_hostent("IPv4",entp);
        }
    }
next1:
    {
        //AF_INET6
        gethostbyname2_r(hostname,AF_INET6,&ent,buf,sizeof(buf),&entp,&herrno);
        if(!entp){
            fprintf(stderr,"gethostbyname2_r(%s,AF_INET6) failed:%s\n",
                    hostname,hstrerror(herrno));
            goto next2;
        }else{
            print_hostent("IPv6",entp);
        }
    }
next2:
    return 0;
}
