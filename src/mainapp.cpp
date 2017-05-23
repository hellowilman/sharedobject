#include "mainapp.h"
#include "cmdline.h"
#include <sharedobjectdata.h>
#include "sharedobject.h"
#include "md5.h"
#include <fstream>
#include <mutex>
#include <stdio.h>

#define _NUM_OF_CLIENTS (1)
SO_STR	mykey = "k5";

static void print(const ValueObject & vo)
{
    vo.p();
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


static void sleep_ms(const long int num){
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
}

static void sleep_sec(const long int num){
    sleep_ms(num*1000);
}

void MainApp::Test_MultiClients2(const int num, const int time_sec, const std::string host)
{
    // create clients
    bool start = true;

    std::string *hexmd5s = new std::string[num];
    std::thread **threads = new std::thread*[num];
    SharedObjectCli * clients = new SharedObjectCli[num];

    std::mutex md5_mutex;

    for(int id = 0; id < num;id++){
        threads[id] = new std::thread([&clients,&start,id,hexmd5s,&md5_mutex,&host](){

            sleep_ms(1000);
            SharedObjectCli& cli = clients[id];
            cli.connect(host);
            cli.sync();
            srand (id);
            int k =0;
            while(start){
                // do setting
                // printf("thread %d working \n", id);
                int r = rand() % 5000;
                std::string key = "worker" + std::to_string(id) + "_" + std::to_string(r);
                std::string val =std::to_string(r);
                cli.set(key,val);
                printf("cli#%d\t set %s\t:\t%s \t%d\n", id, key.c_str(), val.c_str(),k++);
                sleep_ms(100 + r/50);
                if(k %50 == 1){
                    cli.sync();
                }
            }
            sleep_ms(5000); // wait for 5 seconds
            printf("thread %d done! ", id );printf("exit\n");
        });
        if(threads[id]->joinable()){
            printf("Worker %d started\n", id);
        }
        sleep_ms(300);
    }
    sleep_sec(time_sec);
    start = false;
    printf("sleep 10 seconds\n");
    sleep_sec(10);
    // check by hexmd5
    for(int i=0; i < num; i++){
        if(threads[i]->joinable())
            threads[i]->join();
        std::string fln = "test_" + std::to_string(i) + ".txt";
        remove(fln.c_str());
        clients[i].print_info(fln);
    }
    printf("all done\n");
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
