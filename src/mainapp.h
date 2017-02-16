#ifndef MAIN_H
#define MAIN_H


class MainApp
{
public:
    MainApp();
    void Test1();
    void Test2();
    void Test3();
    void test_all();
    void start_srv();
    void start_cli();
	void Test_MultiClients();
	void ActiveClientsCall(int  ClientID);
	void PassiveClientsCall(int  ClientID);
	void ServerCall();

    void Test_MultiClients2(const int num = 5, const int time_sec = 10); // this is a testing by wilman
};

#endif // MAIN_H
