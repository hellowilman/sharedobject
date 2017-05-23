#include "mainapp.h"
#include "cmdline.h"
#include <sharedobjectdata.h>
#include "sharedobject.h"
#include "md5.h"




int main(int argc, char* argv[]){
//	system("pause");

//	MainApp app;
//	app.Test_MultiClients();

    /* */
    cmdline::parser cmd;
    cmd.add("client", 'c',"start as client");
    cmd.add("server",'s',"start as a server");
    cmd.add("multiclient",'m',"donig a multi-client testing");
    cmd.add<int>("clientnum",'n',"the client number", false);
    cmd.add<int>("testtime",'t',"the testing time in sec", false, 10);
    cmd.add<std::string>("host",'h',"the testing time in sec", false, "tcp://127.0.0.1");
    cmd.parse_check(argc,argv);

    MainApp app;

    if(cmd.exist("client")){
        // client
        printf("Start Client!\n");

        app.start_cli();
        return 0;
    }
    if(cmd.exist("server")){
        // server
        printf("Start Server!\n");
        app.start_srv();
        return 0;
    }
    if(cmd.exist("multiclient")){
        // server
        printf("Start multiple thread for testing!\n");

        app.Test_MultiClients2(cmd.get<int>("clientnum"), cmd.get<int>("testtime"),
                               cmd.get<std::string>("host"));
        return 0;
    }

    printf("Please use --help or -? for usage. \n");

    /**/
    return 0;
}

