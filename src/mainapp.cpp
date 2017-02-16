#include "main.h"
#include "cmdline.h"
#include <sharedobjectdata.h>
#include "sharedobject.h"

#define _NUM_OF_CLIENTS (1)
SO_STR	mykey = "k5";

void print(const ValueObject & vo)
{
    vo.p();
}

int main(int argc, char* argv[]){
	system("pause");

	MainApp app;
	app.Test_MultiClients();

	/*
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
	*/
    return 0;
}


MainApp::MainApp()
{
    std::thread t;
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
    client.on("k5",[](const std::string &msg){
        printf("Callback at %s %s\n","",msg.c_str());
    });
    client.set("k5","this is K5");
    client.sync();
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

void MainApp::ServerCall()
{
	//std::this_thread::sleep_for(std::chrono::seconds(5));
	std::cout << "Launched by Server" << std::endl;
	start_srv();
}
void MainApp::ActiveClientsCall(int  ClientID)
{
	std::this_thread::sleep_for(std::chrono::seconds(ClientID *3 + 5));
	std::cout << "Launched by Client" << ClientID << std::endl;

	SharedObjectCli client;
	client.connect();
	client.sync();
	client.on(mykey, [](const std::string &msg) {
		printf("Callback at %s %s\n", "", msg.c_str());
	});

	char buff[_MAX_PATH];
	snprintf(buff, _MAX_PATH, "This is %d", ClientID + 5);
	std::string msg = buff;
	client.set(mykey, msg);

	
	while (1) {
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	
}

void MainApp::PassiveClientsCall(int  ClientID)
{
	std::this_thread::sleep_for(std::chrono::seconds(ClientID * 3 + 5));
	std::cout << "Launched by Client" << ClientID << std::endl;

	SharedObjectCli client;
	client.connect();
	client.sync();
	client.on(mykey, [](const std::string &msg) {
		printf("Callback at %s %s\n", "", msg.c_str());
	});

	std::string msg;
	SO_STR	key = mykey;
	client.get(key, msg);


	while (1) {
		std::this_thread::sleep_for(std::chrono::seconds(100));
	}

}

void MainApp::Test_MultiClients()
{
	std::thread	Server;
	std::thread ActiveClients[_NUM_OF_CLIENTS];
	std::thread PassiveClients[_NUM_OF_CLIENTS];

	Server = std::thread(&MainApp::ServerCall, this);
	
	for (int ClientID = 0; ClientID < _NUM_OF_CLIENTS; ++ClientID)
	{
		ActiveClients[ClientID] = std::thread(&MainApp::ActiveClientsCall, this, ClientID);
	}	
	
	for (int ClientID = 0; ClientID < _NUM_OF_CLIENTS; ++ClientID)
	{
		PassiveClients[ClientID] = std::thread(&MainApp::PassiveClientsCall, this, ClientID+2);
	}
	
	for (int ClientID = 0; ClientID < _NUM_OF_CLIENTS; ++ClientID)
	{
		ActiveClients[ClientID].join();
		PassiveClients[ClientID].join();
	}
	Server.join();
}
