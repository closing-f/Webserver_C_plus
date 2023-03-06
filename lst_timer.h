
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
// #include "../log/log.h"


class tw_timer;//每个事件对应一个tw_timer
struct client_data
{
    sockaddr_in address;
    int sockfd;
    tw_timer *timer;
};
class tw_timer{
    public:
        tw_timer(int rot,int ts):next(NULL ),prev(NULL),
        rotation(rot ),time_slot( ts ){

        }
    public:
        int rotation;//记录定时器还需要转多少轮
        int time_slot;//记录定时器处于哪个时间槽
        void(*cb_func)(client_data*);//?为何cb_func要用指针 定时器回调函数
        client_data* user_data;
        tw_timer*next;
        tw_timer*prev;

};
class time_wheel{
    public:
        time_wheel():cur_slot(0){
            for(int i=0;i<N;i++){
                slots[i]=NULL;
            }
        }
        ~time_wheel(){
            for(int i=0;i<N;i++){
                tw_timer*tmp=slots[i];
                while(tmp){
                    slots[i]=tmp->next;
                    delete tmp;
                    tmp=slots[i];
                }
            }
        }
        //添加一个定时器 timeout为单位为秒,表示多久后会触发
        tw_timer* add_timer(int timeout){
            if(timeout<0){
                return NULL;
            }
            int ticks=0;
            if(timeout<SI){
                ticks=1;
            }
            else{
                ticks=timeout/SI;
            }
            int rotation=ticks/N;

            int time_slot=(cur_slot+ticks%N)%N;
            tw_timer*timer=new tw_timer(rotation,time_slot);
            if(!slots[time_slot]){
                slots[time_slot]=timer;
        
            }else{
                timer->next=slots[time_slot];
                slots[time_slot]->prev=timer;
                slots[time_slot]=timer;

            }
            return timer;


        }
        void del_timer(tw_timer*timer){
            if(!timer){
                return;
            }
            int ts=timer->time_slot;
            if(slots[ts]==timer){
                slots[ts]=timer->next;\
                if(slots[ts]){
                    slots[ts]->prev=NULL;
                }
                delete timer;
            }else{
                timer->prev->next=timer->next;
                if(timer->next){
                    timer->next->prev=timer->prev;
                }
                delete timer;
            }
        }
        void tick(){
            tw_timer* tmp=slots[cur_slot];
            while(tmp){
                if(tmp->rotation>0){
                    tmp->rotation-=1;
                    tmp=tmp->next;
                }else{
                    tmp->cb_func(tmp->user_data);
                    if( tmp == slots[cur_slot]){
                        slots[cur_slot]= tmp->next;
                        delete tmp;
                        if( slots[cur_slot])
                        {
                        slots[cur_slot]->prev = NULL;
                        }
                        tmp = slots[cur_slot];
                    }else
                    {
                        tmp->prev->next = tmp->next;
                        if( tmp->next )
                        {
                        tmp->next->prev = tmp->prev;
                        }
                        tw_timer* tmp2 = tmp->next;
                        delete tmp;
                        tmp = tmp2;

                    }
                }
            }
            cur_slot=++cur_slot%N;
        }
        
    private:

        static const int N = 60;/*时间轮上槽的数目*/
        static const int SI = 1;//每1s时间轮转动一次，即槽间隔为1s
        tw_timer* slots[N];/*时间轮的槽，其中每个元素指向一个定时器链表，链表无序*/
        int cur_slot; /*时间轮的当前槽*/
};
