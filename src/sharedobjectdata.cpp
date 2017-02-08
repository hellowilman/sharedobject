#include "sharedobjectdata.h"
#include <exception>

#define VO_HEADER_SIZE 16
typedef struct{
    NUM ver;
    NUM user;
    NUM time;
    NUM size;
} VoHeader;

typedef struct{
    NUM ver;
    NUM vo_num;
    NUM size;
    NUM reserve;
} SoHeader;

const ValueObject &SharedObjectData::get(const std::string &key)
{
    if(data_.find(key)!=data_.end()){
        return data_[key];
    }else{
        //printf("Cannot find the key %s\n", key.c_str());
        return ValueObject();
    }
}

void SharedObjectData::set(const std::string &key, const ValueObject &vo)
{
    if(key.size()>0){
       if(get(key).ver_ <= vo.ver_){
           //printf("Set OK!: the version is %d-%d\n", get(key).ver_, vo.ver_);
           data_[key] = vo;
       }else{
           //printf("Set failed!: the version is outdate %d-%d\n", get(key).ver_, vo.ver_);
       }
    }
}

const std::string SharedObjectData::toStr() const
{
    // preparing the out
    int sz = sizeof(SoHeader); // header
    for(auto & kv: data_){
        auto& key = kv.first;
        auto& vo = kv.second;
        sz += KVObject::size(key,vo)+sizeof(NUM);
    }
    std::string out(sz,'\0');
    char* pOut = (char*) out.data();
    // prepare header
    SoHeader *pH = (SoHeader*) pOut;
    pH->size = sz - sizeof(SoHeader);
    pH->ver = ver_;
    pH->vo_num = data_.size();
    pH->reserve = 0;
    pOut += sizeof(SoHeader);
    // prepare the kv data
    for(auto &kv:data_){
        auto kvstr = KVObject::toStr(kv.first,kv.second);
        *((NUM*) pOut) = kvstr.size();
        memcpy(pOut+sizeof(NUM), kvstr.data(), kvstr.size());
        pOut += sizeof(NUM) + kvstr.size();
    }
    return out;
}

int SharedObjectData::init(const std::string &data)
{
    // check the data
    const char* pData = data.data();
    int N = data.size();
    const SoHeader *pH = (SoHeader*) pData;
    if(pH->size != N - sizeof(SoHeader) ){
        printf("Cannot initial the SharedObjectData with incorrect data!\n");
    }
    pData += sizeof(SoHeader);
    // processing
    data_.clear();
    ver_ = pH->ver;
    int kv_num = pH->vo_num;
    int rest_num = pH->size;
    //printf("kv_num %d\n", kv_num);
    while(rest_num>0 && kv_num > 0){
        int len = *((NUM*)pData);
        std::string kvstr(pData+sizeof(NUM), len);
        std::string k;
        ValueObject vo;
        if( KVObject::fromStr(kvstr,k,vo) >=0){
            set(k,vo);
        }
        rest_num -= len + sizeof(NUM);
        pData += len + sizeof(NUM);
        kv_num--;
    }
    return pH->vo_num - kv_num;
}

void SharedObjectData::p() const
{
    printf("SharedObject ver: %d\n", ver_);
    for(auto & kv:data_){
        printf("key: %s val: ", kv.first.c_str());
        kv.second.p();
    }
}


ValueObject::ValueObject(const ValueObject &vo)
{
    time_ = vo.time_;
    ver_ = vo.ver_;
    val_ = vo.val_;
    user_ = vo.user_;
}

ValueObject::ValueObject(const std::string &data)
{
    fromStr(data);
}

ValueObject::ValueObject(const std::string &val, NUM ver, NUM user, NUM time)
{
    val_ = val;
    time_ = time;
    ver_ = ver;
    user_ = user;
}

const std::string ValueObject::toStr() const
{
    // prepare data buffer
    NUM sz = size();
    std::string out(sz,'\0');
    // set header
    VoHeader* pheader = (VoHeader*) out.data();
    pheader->size = val_.size();
    pheader->time = time_;
    pheader->user = user_;
    pheader->ver = ver_;
    // set value;
    memcpy((char*)out.data() + sizeof(VoHeader), val_.data(), val_.size());
    return out;
}

int ValueObject::fromStr(const std::string &data)
{
    int N = data.size();
    if(N < sizeof(VoHeader)){
        printf("Cannot initial ValueObject with incorrent data (1)!");
        //throw std::exception("Cannot initial ValueObject with incorrent data (1)!");
        return -1;
    }
    VoHeader * pH = (VoHeader*) data.data();
    if(N != pH->size + sizeof(VoHeader)){
        printf("Cannot initial ValueObject with incorrent data (2)!");
        //throw std::exception("Cannot initial ValueObject with incorrent data (2)!");
        return -2;
    }
    val_ = data.substr(sizeof(VoHeader));
    time_ = pH->time;
    ver_ = pH->ver;
    user_ = pH->user;
    return 0;
}

int ValueObject::size() const
{
    return val_.size() + sizeof(VoHeader);
}

void ValueObject::p() const
{
    printf("time-%d ver-%d user-%d: %s\n", time_,ver_,user_,val_.c_str());
}

std::string KVObject::toStr(const std::string &k, const ValueObject &vo)
{
    const int NUM_BYTES = sizeof(NUM);
    int sz = KVObject::size(k,vo);
    std::string out(sz,0);
    char* pData = (char*)out.data();
    // copy key
    *((NUM*)pData) = k.size();
    memcpy(pData+ NUM_BYTES, k.data(), k.size());
    pData += k.size() + NUM_BYTES;
    // copy VO
    *((NUM*)pData) = vo.size();
    memcpy(pData + NUM_BYTES, vo.toStr().data(), vo.size());

    // return
    return out;
}

int KVObject::fromStr(const std::string &data, std::string &key, ValueObject &vo)
{
    if(data.size() < sizeof(NUM)+2){
        return -1;
    }
    const char* pData = data.data();
    int keysize = *((NUM*) pData);
    if(keysize > data.size() - sizeof(NUM)){
        return -2;
    }
    key = data.substr(sizeof(NUM), keysize);
    int vosize = *((NUM*)(pData + sizeof(NUM) + keysize));
    if(vosize != data.size() - sizeof(NUM)*2 - keysize){
        return -3;
    }
    auto vostr = data.substr(sizeof(NUM)*2+keysize);
    ValueObject vo1(vostr);
    vo = vo1;
    return 0;
}

int KVObject::size(const std::string &k, const ValueObject &vo)
{
    const int NUM_BYTES = sizeof(NUM);
    return k.size() + vo.size() + 2 * NUM_BYTES;
}
