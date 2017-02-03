#include "sharedobject.h"

SharedObjectCli::SharedObjectCli()
{

}

int SharedObjectCli::on(const std::__1::string &key, SharedObjectCli::Callback_Fnc fnc)
{
    if(key.size()){
        callbacks_[key] = fnc;
        return 0;
    }else{
        return -1;
    }
}
