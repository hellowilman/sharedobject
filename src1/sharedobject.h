#ifndef SHAREDOBJECT_H
#define SHAREDOBJECT_H

#include <string>
#include <map>
#include <sharedobjectdata.h>
#define SO_STR std::string

class SharedObjectCli
{
public:
    typedef void(*Callback_Fnc)(const SO_STR &);
    SharedObjectCli();
    int set(const SO_STR& key, const SO_STR & val);
    int get(const SO_STR& key, SO_STR& val);
    int on(const SO_STR& key, Callback_Fnc fnc);

private:
    SharedObjectData so_;
    std::map<SO_STR, Callback_Fnc> callbacks_;
};


class SharedObjectSrv{
public:
    SharedObjectSrv();
private:
    SharedObjectData so_;
};

#endif // SHAREDOBJECT_H
