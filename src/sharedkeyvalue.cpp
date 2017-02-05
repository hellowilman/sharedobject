#include "sharedkeyvalue.h"

typedef const std::map<std::string, std::string> KvMap;

typedef struct{
    char flag;
    char dump[3]; // for mem aligment only
    int ver ;
} MSG_HEADER;

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


SharedKeyValueSrv::SharedKeyValueSrv():
    context(1),
    socket_pub(context, ZMQ_PUB),
    socket_rep(context, ZMQ_REP)
{

}


SharedKeyValueCli::SharedKeyValueCli(int mode):
    context(1),
    socket_sub(context, ZMQ_SUB),
    socket_req(context, ZMQ_REQ),
    thread_sub_flag_(false),
    msg_count_(0)
{

}

int SharedKeyValueCli::get(std::string key, std::string *pval)
{
    return data_.get(key, pval);
}

int SharedKeyValueCli::set(std::string key, std::string value)
{
    int rc = data_.set(key, value);
    if(rc == 0){
        // success and do update
        int buf_size  = sizeof(MSG_HEADER)+MSG_SIZE;
        char * buf = new char[buf_size];

        MSG_HEADER* header = (MSG_HEADER*) buf;
        header->flag = 'u';
        header->ver = data_.ver();
        SharedData::setdata(buf + sizeof(MSG_HEADER),key,value);
        std::string data(buf, buf_size) ;
        s_send(socket_req, data);
        std::string resp = s_recv(socket_req);
        printf("Set resp is %s\n", resp.c_str());

    }else{
        // set data error
        return -1;
    }
    return data_.set(key, value);
}

int SharedKeyValueCli::sync()
{
    return -100;
}

int SharedKeyValueCli::on(std::string key, ShareKeyValueAPI::CallBack func)
{
    if(key == "") return -1;
    callbacks_[key] = func;
    return 0;
}

int SharedKeyValueCli::init(std::string host, int port)
{
    // check host and port

    // disconnect if current is connect
    if(thread_sub_flag_){
        thread_sub_flag_ = false;
    }

    std::string host_ = host;
    int port_ = port;

   // connect to server and listen to the server pub in one thread
   socket_sub.setsockopt(ZMQ_IDENTITY, "Hello", 5);
   socket_sub.setsockopt(ZMQ_SUBSCRIBE, "", 0);
   socket_sub.connect((host_ + ":" + std::to_string(port_)).c_str());
   printf("Sub is on %s\n", (host_+":"+std::to_string(port_)).c_str() );

   socket_req.connect((host_ + ":" + std::to_string(port_+1)).c_str());
   printf("Req is on %s\n", (host_+":"+std::to_string(port_+1)).c_str() );

   thread_sub_flag_ = true;
   msg_count_ = 0;
   std::thread* pThread = new std::thread([&]{this->thread_sub() ;});
   if(pThread->joinable()){
       printf("initial OK\n");
       return 0;
   }else{
       printf("initial Error\n");
       return -1; // error!
   }
}

void SharedKeyValueCli::thread_sub()
{
    while(thread_sub_flag_){
        std::string msg = s_recv(socket_sub);
        printf("SRV MSG(%d):  is %s:\n",msg_count_,  msg.c_str());
        msg_count_++;
    }
}

int SharedKeyValueSrv::init(std::string host,  int port)
{

    std::string host_ = host;
    int port_ = port;

    // bind the host
    socket_pub.bind(host+":"+std::to_string(port));
    printf("Pub is on %s\n", (host+":"+std::to_string(port)).c_str() );
    socket_rep.bind(host+":"+std::to_string(port+1));
    printf("Rep is on %s\n", (host+":"+std::to_string(port+1)).c_str() );
    data_.set("server_id","id1");
    // create a thread to receive data from socket_rep
    auto do_receive = [&](){
        while(1){
            if(flag_start_){
                try{
                    std::string s = s_recv(socket_rep);
                    printf("Received:");printf(" %s\n", s.c_str());
                    process_req(s);
                }catch(std::exception &e){
                    printf("Warning: %s\n", e.what());
                }

                // s_send(socket_pub,"Pub: " + s);
            }else{
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    };
    // need to stop the thread of req
    thread_req = new std::thread( do_receive);
    if(thread_req->joinable()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return 0;
    }else{
        return -1;
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 0;
}

int SharedKeyValueSrv::sync()
{
    int rc; int k = 100;
    while(k-- > 0)
    {
        std::string msg = "Srv said: let us sync! "+std::to_string(k);
        rc = s_send(socket_pub, msg);
        printf( msg.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return -100;
}

int SharedKeyValueSrv::process_req(const std::string &s)
{
    if(s.size() <8){
        printf("not good process\n");
        s_send(socket_rep, "");
        return -1;
    }else{
        printf("Received message: %d bytes \n", s.size());
    }

    MSG_HEADER *p_msg = (MSG_HEADER*)s.data();

    // sync
    std::string ss = "e0000000";
    MSG_HEADER *header =(MSG_HEADER*) ss.data();
    memset(header->dump, ' ', 3);
    std::string ss1;
    std::string resp_msg;
    const char* key,*val;
    switch(p_msg->flag){
    case 's':
        //get a full copy of the current data (sync)
        data_.stringify(&ss1);
        header->flag =  's';
        header->ver = data_.ver();
        resp_msg = ss +  ss1;
        break;

    case 'u':
        //update the copy
        // check data

        if(( (s.size() - sizeof(MSG_HEADER) )% MSG_SIZE )!= 0){

            resp_msg = ss + "format error!";
             if( s_send(socket_pub,ss + "what the half")){
                 printf("error! send pub ");
             }else{
                 printf("error! not send pub ");
             }

        }else{

            key = (s.data()+sizeof(MSG_HEADER));
            val =key + MSG_KEY_SIZE;

            printf("Update: '%s' -'%s' \n",  key, val);

            data_.set(key,val);
            header->flag =  'u';
            header->ver = data_.ver();
            ss1.resize(MSG_SIZE);
            memset((char*)ss1.data(), 0, MSG_SIZE);
            SharedData::setdata((char*)ss1.data(), key, val);
            s_send(socket_pub,ss + ss1);

             resp_msg = ss + ss1;
        }

        break;
    default :
        resp_msg =ss + "format error!";
        printf("send data  Got\n");
        break;
    }
    s_send(socket_rep, resp_msg);
    return 0;
}


