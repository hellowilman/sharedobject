#ifndef SHAREDOBJECT_H
#define SHAREDOBJECT_H

#include <string>
#include <map>
#include <sharedobjectdata.h>
#include <zmq.hpp>
#include <thread>

#include "blockqueue.h"

#define SO_STR std::string
class SharedObjectMsg{
public:
    SharedObjectMsg(){}
    const std::string toStr() const;
    unsigned int size() const;
    int fromStr(const std::string &data);
    int send_by(zmq::socket_t & socket);
public:
    NUM msg_magic_ = 9527; // a magic number (flag0)
    NUM msg_act_ = 0; // the message type number (flag1)
    NUM send_id_ = 0;
    NUM recv_id_ = 0;
    NUM ver_ = 0;
    NUM r1_ = 0,r2_ = 0;
    std::string data_;
    enum flag_act{SET = 1, SET_RESP, SYNC, SYNC_RESP, OUT_DATE, MISC, ACK};
    enum msg_type{MSG_SOD,MSG_KV};
    void p();
};

class SharedObjectCli
{
public:
    typedef void(*Callback_Fnc)(const SO_STR &);
    SharedObjectCli(int id=1);
    int set(const SO_STR& key, const SO_STR & val);
    int get(const SO_STR& key, SO_STR& val);
    int on(const SO_STR& key, Callback_Fnc fnc);
    int connect(std::string host="tcp://127.0.0.1", int port = 10086);

private:
    SharedObjectData so_;
    std::map<SO_STR, Callback_Fnc> callbacks_;
    NUM id_;

    zmq::context_t context_;
    zmq::socket_t  socket_sub_;
    zmq::socket_t  socket_req_;
    int base_port_;
    std::string server_host_;

    std::thread *thread_process_subs_;
    std::thread *thread_process_subq_;
    std::thread *thread_process_reqq_;

    void thread_for_subsocket();
    void thread_for_subqueue();
    void thread_for_reqqueue();

    unsigned int msg_count_ = 0;
    BlockingQueue<SharedObjectMsg> queue_sub;
    BlockingQueue<SharedObjectMsg> queue_req;

    int process_somsg(const SharedObjectMsg& somsg);
    int process_somsg(const std::string & sostr);
    int process_setresp(const SharedObjectMsg&somsg);
    int process_syncresp(const SharedObjectMsg & somsg);
    int process_outdate(const SharedObjectMsg & somsg);

};


class SharedObjectSrv{
public:
    SharedObjectSrv();
    int bind(const std::string host="tcp://127.0.0.1", int port = 10086);

private:
    SharedObjectData so_;
    zmq::context_t context;
    zmq::socket_t  socket_pub;
    zmq::socket_t  socket_resp;
    NUM id_ = 0;
};

#endif // SHAREDOBJECT_H
