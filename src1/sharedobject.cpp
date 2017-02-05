#include "sharedobject.h"
#include <time.h>
static bool
s_send (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());
    bool rc = socket.send (message);
    return (rc);
}

static std::string
s_recv (zmq::socket_t & socket) {

    zmq::message_t message;
    socket.recv(&message);

    return std::string(static_cast<char*>(message.data()), message.size());
}

typedef SharedObjectMsg SOMsg;

SharedObjectCli::SharedObjectCli(int id):
    context_(1),
    socket_req_(context_, zmq::socket_type::req),
    socket_sub_(context_,zmq::socket_type::sub)
{
    thread_process_subs_ = nullptr;
    id_ = id;
}
static long int time_stamp(){
    return time(NULL);
}

int SharedObjectCli::set(const std::string &key, const std::string &val)
{
    // set the key
    ValueObject vo(val, so_.getVer(), id_, time_stamp());
    SOMsg somsg;
    somsg.msg_act_ = SOMsg::SET;
    somsg.send_id_ = id_; // myself
    somsg.recv_id_ = 0; // server
    somsg.data_ = KVObject::toStr(key,vo);
    queue_req.put(somsg);
}

int SharedObjectCli::get(const std::string &key, std::string &val)
{
    auto vo = so_.get(key);
    val  = vo.val_;
    return 0;
}

int SharedObjectCli::on(const std::__1::string &key, Fn fnc)
{
    if(key.size()){
        callbacks_[key] = fnc;
        return 0;
    }else{
        return -1;
    }
}

int SharedObjectCli::connect(std::string host, int port)
{
    base_port_ = port;
    server_host_ = host;
    std::string id_str = "CLI"+std::to_string(id_);
    socket_sub_.setsockopt(ZMQ_IDENTITY, id_str.c_str(), id_str.size());
    socket_sub_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    socket_sub_.connect(server_host_+":"+std::to_string(base_port_));
    socket_req_.connect(server_host_+":"+std::to_string(base_port_+1));
    printf("Sub is on %s\n", (server_host_+":"+std::to_string(base_port_)).c_str() );
    printf("Req is on %s\n", (server_host_+":"+std::to_string(base_port_+1)).c_str() );
    if(!thread_process_subs_){
        thread_process_subs_ = new std::thread([this](){
            this->process_subsocket();
        });
    }
    printf("Start thread for subsocket receiving \n");
    if(!thread_process_subq_){
        thread_process_subq_ = new std::thread([this](){
            this->process_subqueue();
        });
    }
    printf("Start thread for sub queue processing \n");

    if(!thread_process_reqq_){
        thread_process_reqq_ = new std::thread([this](){
            this->process_reqqueue();
        });
    }
    printf("Start thread for req queue processing \n");

    if(! thread_process_subs_->joinable() ){
        printf("thread_process_subs_ faild!");
        return -1;
    }
    if(! thread_process_subq_->joinable() ){
        printf("thread_process_subq_ faild!");
        return -1;
    }
    if(! thread_process_reqq_->joinable() ){
        printf("thread_process_reqq_ faild!");
        return -1;
    }
    printf("connect successed! \n");
    return 0;
}

void SharedObjectCli::process_subsocket()
{
    try{
        while(1){
            std::string msg = s_recv(socket_sub_);

            msg_count_++;
            SharedObjectMsg somsg;
            if(somsg.fromStr(msg) >= 0){
                queue_sub.put(somsg);
            }else{
                printf("Incorrect message received from sub channel\n");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep for a small ms
        }
    }catch(std::exception &e){
        printf("Error on sub: %s\n", e.what());
    }
}

void SharedObjectCli::process_subqueue()
{
    try{
        while(1){
            SharedObjectMsg somsg = queue_sub.take();
            printf("Sub Queue Msg %d:%s\n", somsg.ver_, somsg.data_.c_str());
            process_somsg(somsg);
        }
    }catch(std::exception &e){
        printf("Error on sub: %s\n", e.what());
    }
}

void SharedObjectCli::process_reqqueue()
{
    try{
        while(1){
            SharedObjectMsg somsg = queue_req.take();
            somsg.send_by(socket_req_);
            std::string recv = s_recv(socket_req_);
            process_somsg(recv);
        }
    }catch(std::exception &e){
        printf("Error on req: %s\n", e.what());
    }
}

int SharedObjectCli::process_somsg(const std::string &sostr)
{
    SOMsg somsg;
    int rc = somsg.fromStr(sostr);
    if(rc <0){
        return -1;
    }
    return process_somsg(somsg);
}

int SharedObjectCli::process_somsg(const SharedObjectMsg &somsg)
{
    int rc = 0;
    switch(somsg.msg_act_){
    case SharedObjectMsg::SET_RESP:
        rc = process_setresp(somsg);
        break;
    case SharedObjectMsg::SYNC_RESP:
        rc = process_syncresp(somsg);
        break;

    case SharedObjectMsg::OUT_DATE:

        break;
    case SharedObjectMsg::ACK:
        // ack, no need to do update
        break;
    }
    return rc;
}

int SharedObjectCli::process_setresp(const SharedObjectMsg & somsg)
{
    ValueObject vo;
    std::string k;
    KVObject::fromStr(somsg.data_, k, vo);
    somsg.p();
    printf("insert k-v: %s-%s\n",k.c_str(), vo.val_.c_str());
    so_.set(k,vo);

    // call the update callback
    if(callbacks_.find(k) != callbacks_.end()){
        callbacks_[k](vo.val_);
    }else{
        printf("No callback for %s\n", k.c_str());
    }
    return 0;
}

int SharedObjectCli::process_syncresp(const SharedObjectMsg & somsg )
{
    if( so_.init(somsg.data_) >=0){
        return 0;
    } else{
        printf("Incorrect sync-resp data\n");
        return -1;
    }
}

int SharedObjectCli::process_outdate(const SharedObjectMsg &somsg)
{
    // outdate so get update from server
    SOMsg msg;
    msg.msg_act_ = SOMsg::flag_act::SYNC;
    msg.send_id_ = id_;
    msg.recv_id_ = 0; // means server
    msg.ver_ = so_.getVer();
    msg.data_ = "";

    return -100;
}
typedef struct{
    NUM flag_magic;
    NUM send,recv;
    NUM ver;
    NUM flag_act;
    NUM size;
    NUM r1,r2; // reserved
} SOMsgHeader;
unsigned int SharedObjectMsg::size() const
{
    return data_.size() + sizeof(SOMsgHeader);
}

const std::string SharedObjectMsg::toStr() const
{
    // get the size and prepare the data buffer
    int sz = size() ;
    std::string out(sz,0);
    char* pData = (char*) out.data();
    // set the header
    SOMsgHeader *pH = (SOMsgHeader*) pData;
    pH->flag_magic = msg_magic_;
    pH->flag_act = msg_act_;
    pH->send  = send_id_;
    pH->recv = recv_id_;
    pH->size = data_.size();
    pH->ver = ver_;
    pH->r1 = r1_;
    pH->r2 = r2_;
    // set the body
    memcpy(pData+sizeof(SOMsgHeader), data_.data(), data_.size());
    return out;
}


int SharedObjectMsg::fromStr(const std::string &data)
{
    if(data.size() <= sizeof(SOMsgHeader)){
        return -1;
    }
    const char* pData = data.data();
    SOMsgHeader &pH = * ( (SOMsgHeader*) pData );
    if(pH.size + sizeof(SOMsgHeader) != data.size()){
        printf("Incorrent SharedObjectMsg data\n");
        return -2;
    }

    msg_magic_ = pH.flag_magic;
    msg_act_ = pH.flag_act;
    r1_ = pH.r1;
    r2_ = pH.r2;
    ver_ = pH.ver;
    send_id_ = pH.send;
    recv_id_ = pH.recv;
    data_.resize(pH.size, 0);
    memcpy((char*)data_.data(), pData + sizeof(SOMsgHeader), pH.size);
    return 0;
}

int SharedObjectMsg::send_by(zmq::socket_t &socket)
{
    if( s_send(socket,toStr()) ){
        return 0;
    }else{
        // cannot send via socket
        printf("SharedObjectMsg: Cannot send via socket\n");
        return -1;
    }

}


void SharedObjectMsg::p() const
{
    printf("v%d s%d r%d m%d t%d r1%d r2%d: %s\n", ver_, send_id_, recv_id_, msg_magic_, msg_act_, r1_, r2_, data_.c_str());
}

SharedObjectSrv::SharedObjectSrv():
    context(1),
    socket_pub(context,ZMQ_PUB),
    socket_resp(context,ZMQ_REP)
{

}

int SharedObjectSrv::bind(const std::__1::string host, int port)
{
    socket_pub.bind(host+":"+std::to_string(port));
    socket_resp.bind(host+":"+std::to_string(port+1));
    printf("pub on %s\n", (host+":"+std::to_string(port)).c_str());
    printf("resp on %s\n", (host+":"+std::to_string(port+1)).c_str());
    // start the thread



    // do something
    new std::thread([this]{
        process_testing();
    });
}

void SharedObjectSrv::process_testing()
{
    int k = 0;
    while( ++k ){

        time_t t = time(NULL);
        printf("Pub # %d Time is %d\n", k,t );
        ValueObject vo("key is k"+std::to_string(k),10000+k,0, t);
        std::string key = "k"+std::to_string(k);
        so_.set(key, ValueObject("",10000+k,0, t));
        SOMsg msg;
        msg.send_id_ = 0;
        msg.recv_id_ = ~0; // all

        if(k%10 == 0){
            msg.msg_act_ = SOMsg::SYNC_RESP;
            msg.data_ = so_.toStr();
        }else{
            msg.msg_act_ = SOMsg::SET_RESP;
            msg.data_ = KVObject::toStr(key, vo);
            msg.r1_ = SOMsg::MSG_KV;
        }
        if(k%30==29){
            k = 0;
        }
        msg.ver_ = 10000+k;
        msg.p();
        msg.send_by(socket_pub);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void SharedObjectSrv::process_respsocket()
{
    while(1){
        try{
            std::string reqstr = s_recv(socket_resp);
            SOMsg somsg, resp_msg;
            int rc = somsg.fromStr(reqstr);

            if(rc >=0){
                if(somsg.msg_act_ == SOMsg::SET){
                    // update so_
                    ValueObject vo;
                    SO_STR key;
                    rc = KVObject::fromStr(somsg.data_, key,vo);
                    if(rc >=0){
                        setnpub(key,vo);
                        // prepare msg response to requester
                        resp_msg.msg_act_ = SOMsg::ACK;
                        resp_msg.data_ = "";
                    }
                }
                if(somsg.msg_act_ == SOMsg::SYNC){
                    resp_msg.data_ = so_.toStr();
                    resp_msg.msg_act_ = SOMsg::SYNC_RESP;
                }
            }
            resp_msg.ver_ = so_.getVer();
            resp_msg.send_id_ = id_;
            resp_msg.recv_id_ = somsg.send_id_;
            resp_msg.send_by(socket_resp);
        }catch(std::exception &e){
            printf("process resp socket error: %s \n", e.what());
        }
    }
}

int SharedObjectSrv::setnpub(const std::__1::string &key, ValueObject &vo)
{
    // update so_
    if(vo.ver_ > so_.get(key).ver_){
        so_.setVer(so_.getVer()+1);
        vo.ver_ = so_.getVer();
        vo.time_ = time_stamp();
        so_.set(key,vo);
    }
    // pub the update
    SOMsg pubmsg;
    pubmsg.ver_ = so_.getVer();
    pubmsg.send_id_ = id_;
    pubmsg.recv_id_ = ~0;
    pubmsg.msg_act_ = SOMsg::SET_RESP;
    pubmsg.data_ = KVObject::toStr(key,vo);
    pubmsg.send_by(socket_pub);
}
