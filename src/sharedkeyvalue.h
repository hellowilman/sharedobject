#ifndef SHAREDKEYVALUE_H
#define SHAREDKEYVALUE_H
#include<string>
#include <zmq.hpp>
#include <map>
#include <mutex>
#include <thread>

#define MSG_SIZE 128
#define MSG_KEY_SIZE 16

class ShareKeyValueAPI{
public:
    typedef void (*CallBack)(std::string &);
    virtual int init(std::string host, int port = 16578) = 0;
    virtual int get(std::string key, std::string *pval) = 0;
    virtual int set(std::string key, std::string value) = 0;
    virtual int sync() = 0;
    virtual int on(std::string key, CallBack func) = 0;

};

class ShareKeyValueSrvAPI{
public:
    virtual int start(int start_flag = 1) = 0;
    virtual int init( std::string host = "tcp://0.0.0.0", int port = 16578) = 0;
    virtual int sync() = 0;
};
class SharedData{
public:
    int set(const std::string key, std::string val){
        std::lock_guard<std::mutex> lock(data_mutex_);
        if(key.size() > MSG_KEY_SIZE || val.size() > MSG_SIZE-MSG_KEY_SIZE){
            return -2;
        }
        data_[key] = val;
        ver_++;
        return 0;
    }
    int get(const std::string key_, std::string *val){
        std::lock_guard<std::mutex> lock(data_mutex_);
        std::string key = key_;
        if(key.size()>MSG_KEY_SIZE){
            key = key.substr(0,MSG_KEY_SIZE);
        }
        if(val && data_.find(key)!=data_.end()){
            (*val) = data_[key];
        }else{
            return -1;
        }
    }
    int parse(const std::string& str){
        if(str.size()%MSG_SIZE != 0 ){
            return -1;// parse error!
        }else{
            int N = str.size()/MSG_SIZE;
            for(int i =0; i <N; i++){
                std::string key, val;
                key = str.substr(i*MSG_SIZE, MSG_KEY_SIZE);
                val = str.substr(i*MSG_SIZE+MSG_KEY_SIZE, MSG_SIZE-MSG_KEY_SIZE);
                if(set(key, val) <0){
                    printf("warning: insert key error!\n");
                };
            }
        }
    }
    static int setdata(char* buf, const std::string &key, const std::string &val){
        printf("key size is %d\n", key.size());
        memcpy(buf , key.data(), key.size());
        memcpy(buf +MSG_KEY_SIZE, val.data(), val.size());
        return 0;
    }
    int stringify(std::string *str){
        std::lock_guard<std::mutex> lock(data_mutex_);
        if(!str){
            return -1;
        }
        int N = data_.size();
        char *buf = new char[N * MSG_SIZE];
        memset(buf, 0, N *MSG_SIZE );
        if(buf){
            int id  = 0;
            for(auto & d:data_){
                std::string key, val;
                key = d.first;
                val = d.second;
                //TODO: there is  some bugs here (write the out of boundary memory)
                setdata(buf+id*MSG_SIZE, key,val);
                id++;
            }
            std::string data(buf, N*MSG_SIZE);
            data.swap((*str));
            delete[] buf;
            return 0;
        }else{
            return -2; // out of memory
        }
    }
    int ver(){return ver_;};
private:
    std::map<std::string, std::string > data_;
    std::mutex data_mutex_;
    int ver_ = 0;
};

class SharedKeyValueSrv:public ShareKeyValueSrvAPI{
public:
    SharedKeyValueSrv();
    // ShareKeyValueSrvAPI interface
public:
    int start(int start_flag){ flag_start_ = start_flag;  return 0; }
    int init( std::string host = "tcp://*", int port = 16578);
    int sync();
private:
    zmq::context_t context;
    zmq::socket_t socket_pub;
    zmq::socket_t socket_rep;

    int process_req(const std::string & s);

    bool flag_start_ = false;
    std::thread *thread_req;
    std::string host_;
    int port_;
    SharedData data_;
};

class SharedKeyValue:public ShareKeyValueAPI, public ShareKeyValueSrvAPI
{
public:
    SharedKeyValue(int mode = 0);

    // ShareKeyValueAPI interface
public:
    int get(std::string key, std::string *pval);
    int set(std::string key, std::string value);
    int sync();
    int on(std::string key, CallBack func);
    int init(std::string host, int port);

private:
    zmq::context_t context;

    zmq::socket_t socket_sub;
    zmq::socket_t socket_req; // client

    SharedData data_;
    std::map<std::string, CallBack> callbacks_;
    std::mutex data_mutex_;

    std::string host_;
    int port_;

};

#endif // SHAREDKEYVALUE_H
