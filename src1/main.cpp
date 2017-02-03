#include "main.h"
#include "cmdline.h"
#include <sharedobjectdata.h>

void print(const ValueObject & vo)
{
    vo.p();
}

int main(int argc, char* argv[]){
    cmdline::parser cmd;
    cmd.add("client", 'c',"start as client");
    cmd.add("server",'s',"start as a server");
    cmd.parse_check(argc,argv);

    ValueObject vo("valu0123456789",1,1,100);
    print(vo);
    std::string data = vo.toStr();
    ValueObject vo2(data);
    print(vo2);

    printf("Testing 2 ---- >>>>>> \n");

    SharedObjectData sod;
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
    return 0;
}


MainApp::MainApp()
{

}

void MainApp::start()
{

}
