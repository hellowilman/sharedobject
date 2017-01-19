#include "zmq.hpp"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "sharedkeyvalue.h"



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


int main(int argc, char *argv[])
{
//    if(argc >1){
//        start_sub();
//    }else{
//        start_pub();
//    }
    SharedKeyValueSrv srv;
    srv.init();


    srv.start(true);
    std::this_thread::sleep_for(std::chrono::minutes(60));

    return 1;
}

