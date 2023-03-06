

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"
#include "sql_connection_pool.h"
template<typename T>
class threadpool{
public:
    threadpool(int thread_number=8,int max_request_number=10000);
    ~threadpool();
    bool append(T*request);
private:
    static void* worker(void*arg);
    void run();
private:
    int m_thread_number;//线程池中线程数
    int m_max_request_number;//线程池中的最大请求数目
    pthread_t *m_threads;//线程池数组
    std::list<T*>m_workqueue;//工作请求队列
    locker m_queuelock;//保护请求队列
    sem m_queue_state;//工作队列信号量
    bool m_stop;// 是否结束线程
};

template< typename T>
threadpool<T>::threadpool(int thread_number,int max_request_number):m_thread_number(thread_number),
m_max_request_number(max_request_number),m_threads(NULL),m_stop(false){
    if(thread_number<=0||max_request_number<=0){
        throw std::exception();

    }

    m_threads=new pthread_t[m_thread_number];
    if(!m_threads)
        throw std::exception();
    for(int i=0;i<m_thread_number;i++){
        if(pthread_create(m_threads+i,NULL,worker,this)!=0){
            delete []m_threads;
            throw std::exception();

        }
        if(pthread_detach(m_threads[i])){
            delete []m_threads;
            throw std::exception();

        }
    }


}
template<typename T>
threadpool<T>::~threadpool(){
    delete []m_threads;
    m_stop=1;
}

template<typename T>
bool threadpool<T>::append(T*request){
    m_queuelock.lock();
    if(m_workqueue.size()>m_max_request_number){
        m_queuelock.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelock.unlock();
    m_queue_state.post();
    return true;
}

template<typename T>
void * threadpool<T>::worker(void*arg){
    threadpool* pool=(threadpool*)arg;
    pool->run();
    return pool;//? 返回pool干嘛

}

template<typename T>
void threadpool<T>::run(){
    while(!m_stop){
        m_queue_state.wait();
        m_queuelock.lock();
        if(m_workqueue.empty()){ //?signal之后为何队列可能为空
            m_queuelock.unlock();
            continue;
        }
        T*request=m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelock.unlock;
        if(!request){
            continue;
        }
        request->process();

    }
}