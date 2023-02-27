#include<sys/socket.h>
#include <netinet/in.h>
#include<string.h>
#include<assert.h>
#include <fcntl.h>
#include<sys/epoll.h>
#include<http_conn.h>
const int MAX_FD=65536;
const int MAX_EVENT_NUMBER=10000;
const int TIMESLOT=5;

class Server
{
private:
    /* data */
public:
    Server(/* args */);
    ~Server();
    void init(int port,int user,int password,int listenMode,int conMode,int thread_num,int sql_num);
    int setnonblocking(int fd);
    void thread_pool();
    void sql_pool();
    void eventListen();
    void eventLoop();
    void addfd2epoll(int epollfd,int fd,bool if_oneshot);
    bool dealclientdata();
public:
    int server_port;
    int server_listenMode;
    int server_conMode;
    int server_thread_num;
    int server_sql_num;
    int s_listenfd;
    int s_epollfd;
    int s_pipefd[2];
    epoll_event events[MAX_EVENT_NUMBER];
};