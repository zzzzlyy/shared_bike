#include<iostream>
#include<unistd.h>
#include<memory>
#include"bike.pb.h"
#include"event.h"
#include"events_def.h"
#include"user_event_hander.h"
#include"DispatchMsgService.h"
#include"NetworkInterface.h"
#include"iniconfig.h"
#include"Logger.h"
#include"sqlconnection.h"
#include"SqlTables.h"
#include"BusProcessor.h"
using namespace std;

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("please input shbk <config file path> <log file config>!\n");
        return -1;
    }
    if (!Logger::instance()->init(std::string(argv[2])))
    {
        //Logger::instance()->GetHandle()->error("1111");
        fprintf(stderr, "init log moudle failed.\n");
        return -2;
    }
    Iniconfig *config = Iniconfig::getInstance();
    if (!config->loadfile(std::string(argv[1]))) {
        printf("load %s failed\n", argv[1]);
        LOG_ERROR("load %s failed\n", argv[1]);
        return -3;
    }
    st_env_config conf_args = config->getconfig();
    LOG_INFO("[database] ip %s port %d\n", conf_args.db_ip.c_str(), conf_args.db_port);


    std::shared_ptr<MysqlConnection>mysqlConn(new MysqlConnection);
    if (!mysqlConn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
        conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
    {
        LOG_ERROR("Database init failed, exit!\n");
        return -4;
    }//初始化数据库
    BusinessProcessor busPro(mysqlConn);
    busPro.init();//初始化表



    tutorial::mobile_request msg;
    msg.set_mobile("13037129121");
    iEvent *ie = new iEvent(EEVENTID_GET_MOBILE_CODE_REQ, 2);
    MobileCodeReqEv me("13037129121");
    //me.dump(cout);
    //cout << msg.mobile() << endl;
    MobileCodeRspEv mcre(200, 66666);
    //mcre.dump(cout);
    //UserEventHandler ueh1;
    //ueh1.handle(&me);
    DispatchMsgService* DMS = DispatchMsgService::getInstance();
    DMS->open();
    NetworkInterface* NTIF = new NetworkInterface();
    NTIF->start(conf_args.db_port);
    while (1)
    {
        NTIF->network_event_dispatch();
        sleep(1);
       LOG_INFO("network_event_dispatch....\n");
    }
    /*
    MobileCodeReqEv* pmcre = new MobileCodeReqEv("12345");
    DMS->enqueue(pmcre);
    */
    sleep(5);
    DMS->close();
    sleep(5);
    return 0;
}