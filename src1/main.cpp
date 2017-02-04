#include "main.h"
#include "cmdline.h"
#include <sharedobjectdata.h>
#include "sharedobject.h"
void print(const ValueObject & vo)
{
    vo.p();
}

int main(int argc, char* argv[]){
    cmdline::parser cmd;
    cmd.add("client", 'c',"start as client");
    cmd.add("server",'s',"start as a server");
    cmd.parse_check(argc,argv);
    if(cmd.exist("client")){
        // client
        printf("Start Client!\n");
        MainApp app;
        app.start_cli();

    }else
    if(cmd.exist("server")){
        // server
        printf("Start Server!\n");
        MainApp app;
        app.start_srv();

    }else{
        printf("Please use --help or -? for usage. \n");
    }

    return 0;
}


MainApp::MainApp()
{

}

void MainApp::Test1()
{
    ValueObject vo("valu0123456789",1,1,100);
    print(vo);
    std::string data = vo.toStr();
    ValueObject vo2(data);
    print(vo2);

}

void MainApp::Test2()
{
    printf("Testing 2 ---- >>>>>> \n");

    SharedObjectData sod;
    ValueObject vo, vo2;
    sod.set("key1",vo);
    sod.set("key2",vo2);
    for(int i=0; i <10; i++){
        vo.val_ += "a";
        sod.set("k"+std::to_string(i), vo);
    }
    std::string sostr = sod.toStr();
    sod.p();
    printf("-----------------------------------\n");
    SharedObjectData sod1;
    sod1.init(sostr);
    sod1.p();
}

void MainApp::Test3()
{
    printf("Testing 3 -------------------------\n");
    SharedObjectMsg smsg;
    smsg.r1_ = 1;
    smsg.r2_ = 2;
    smsg.ver_ = 3;
    smsg.msg_act_ = smsg.SET ;
    smsg.send_id_ = 101;
    smsg.recv_id_ = 102;
    smsg.data_ = "This is message for msg_type";

    std::string sostr = smsg.toStr();
    SharedObjectMsg smsg2;
    smsg2.fromStr(sostr);
    smsg.p();
    printf("-----------------------------------\n");
    smsg2.p();
}

void MainApp::start_srv()
{
    SharedObjectSrv server;
    server.bind();
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void MainApp::start_cli()
{
    SharedObjectCli client;
    client.connect();
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void MainApp::test_all()
{
    Test1();
    Test2();
    Test3();
}
