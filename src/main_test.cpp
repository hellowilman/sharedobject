#include "zmq.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string>


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
    SharedKeyValueCli cli;
    cli.init("tcp://127.0.0.1");
    std::string k,v;
    k = "tmp";
    int num = 0;
    while(num++<10){
        try{
            k= "key" +  std::to_string(num);
            v = "value"+std::to_string(num);
            cli.set(k,v);
        }catch(std::exception &e){
            std::cout << e.what() << std::endl;
        }
    }

    std::this_thread::sleep_for(std::chrono::minutes(60));

    return 1;
}

