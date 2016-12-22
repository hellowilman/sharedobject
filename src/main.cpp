#include "zmq.hpp"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "sharedkeyvalue.h"
void start_pub(){
    zmq::context_t context(1);

    //  Subscriber tells us when it's ready here
    zmq::socket_t sync(context, ZMQ_PULL);
    sync.bind("tcp://*:5564");

    //  We send updates via this socket
    zmq::socket_t publisher (context, ZMQ_PUB);
    publisher.bind("tcp://0.0.0.0:5565");

    while(true){

           //  Wait for synchronization request
           zmq::message_t msg;
           printf("Start to recv msg\n");
           sync.recv(&msg);
           printf("Received %s\n", (char*)msg.data());

           //  Now broadcast exactly 10 updates with pause
           int update_nbr;
           for (update_nbr = 0; update_nbr < 10; update_nbr++) {

               std::string str;
               str = "Update " + std::to_string(update_nbr);
               zmq::message_t m_(str.size());
               memcpy(m_.data(), str.data(), str.size());
               int rc =  publisher.send(m_);
               printf("rc %d: %s ", rc, m_.data());

           }
           printf("\n");

   }
}


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

void start_sub(){
        zmq::context_t context(1);

        //  Connect our subscriber socket
        zmq::socket_t subscriber (context, ZMQ_SUB);
        subscriber.setsockopt(ZMQ_IDENTITY, "Hello", 5);
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        subscriber.connect("tcp://localhost:5565");

        //  Synchronize with publisher
        zmq::socket_t sync (context, ZMQ_PUSH);
        sync.connect("tcp://localhost:5564");
        s_send (sync, "");

        //  Get updates, expect random Ctrl-C death
        while (1) {
            std::string string = s_recv (subscriber);
            printf("%s\n" , string.c_str());

            if (string.compare("END") == 0) {
                break;
            }
        }
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

