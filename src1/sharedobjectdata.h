#ifndef SHAREDOBJECTDATA_H
#define SHAREDOBJECTDATA_H
#include <string>
#include <map>

#define NUM unsigned int

class ValueObject{
public:
    ValueObject():ver_(0),user_(0),time_(0){}
    ValueObject(const ValueObject & vo);
    ValueObject(const std::string &data);
    ValueObject(const std::string &val, NUM ver, NUM user, NUM time);
    const std::string toStr() const;
    int fromStr(const std::string &data);
    int size() const;
public:
    void p() const;
public:
    std::string val_;
    NUM ver_;
    NUM user_;
    NUM time_;
};

class KVObject{
public:
    static std::string toStr(const std::string &k, const ValueObject& vo);
    static int fromStr(const std::string& data, std::string &key, ValueObject &vo);
    static int size(const std::string &k, const ValueObject& vo);
};

class SharedObjectData
{
public:
    SharedObjectData():ver_(0){}
    const ValueObject & get(const std::string & key);
    void set(const std::string &key, const ValueObject &vo);
    const std::string toStr() const;
    int init(const std::string &data);
    void clear(){data_.clear();}
public:
    void p(); // print it;
    unsigned int getVer() const{return ver_;}
    void setVer(unsigned int ver) {ver_ = ver;}

private:
    std::map<std::string, ValueObject> data_;
    NUM ver_;
};

#endif // SHAREDOBJECTDATA_H
