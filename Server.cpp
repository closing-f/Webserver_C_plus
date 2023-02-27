#include<Server.h>


Server::Server(){

};
void Server::init(int port,int user,int password,int listenMode,int conMode,int thread_num,int sql_num ){
    server_port=port;
    server_conMode=conMode;
    server_listenMode=listenMode;
    server_sql_num=sql_num;
    server_thread_num=thread_num;

};
void Server::eventListen(){
    s_listenfd=socket(PF_INET,SOCK_STREAM,0);
    //?
    struct linger tmp ={0,1};
    setsockopt(s_listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));

    int ret=0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=server_port;
    int flag=1;
    setsockopt(s_listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    //? 为何要将sockaddr_in转成sockaddr
    ret=bind(s_listenfd,(struct sockaddr *)&address,sizeof(&address));
    assert(ret>=0);
    ret=listen(s_listenfd,5);
    assert(ret>=0);
    //? 为何要定义两遍events
    epoll_event events[MAX_EVENT_NUMBER];
    s_epollfd=epoll_create(4);
    
    addfd2epoll(s_epollfd,s_listenfd,true);

    http_conn::h_epollfd=s_epollfd;
    // ret= socketpair(PF_LOCAL,SOCK_STREAM,0, s_pipefd);
    // assert(ret!=-1);
    // setnonblocking(s_pipefd[1]);
    // addfd2epoll(s_epollfd,s_pipefd[0],false);
    
    
};
void Server::eventLoop(){
    while(1){
        //? -1是为啥
        int number=epoll_wait(s_epollfd,events,MAX_EVENT_NUMBER,-1);
        if(number<0){
            break;
        }
        for(int i=0;i<number;i++){
            int socketfd=events[i].data.fd;
            if(socketfd==s_listenfd){
                bool flag=dealclientdata();
                //? 为什么要continue?
                if(false==flag)
                    continue;
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                //todo 处理连接关闭
            }
            else if(events[i].events& EPOLLIN){
                //todo 处理读事件
            }
            else if(events[i].events&EPOLLOUT){
                //todo 处理写事件
            }

        }
            

    }

};
bool Server::dealclientdata(){

}
int Server::setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
};
void Server::addfd2epoll(int epollfd,int fd,bool if_oneshot){
    epoll_event event_tmp;
    event_tmp.data.fd=fd;
    //? 为什么要加EPOLLHUP
    event_tmp.events= EPOLLIN | EPOLLHUP;
    if(if_oneshot)
        event_tmp.events |= EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event_tmp);
    setnonblocking(fd);

};


