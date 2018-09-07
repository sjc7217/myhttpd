#include "header.h"


#ifndef _CONFIG_H
#define _CONFIG_H


class Config {
public:
	Config();

	~Config();

	int InitPara(int argc, char *argv[]);

public:
	typedef std::vector<std::string> plugin_list_t;

	std::string CGIRoot;        //CGI根路径
	std::string DefaultFile;    /*默认文件名称*/
	std::string DocumentRoot;       /*根文件路径*/
	std::string ConfigFile;        /*配置文件路径和名称*/
	std::string ListenIP;           /*绑定地址*/
	int ListenPort;                    /*侦听端口*/
	int MaxWorker;            /*最大worker数量*/
	int TimeOut;                    /*超时时间*/
	int InitConPool;            /*初始化连接池大小*/
	plugin_list_t PluginList;       /*插件列表*/
    bool Daemonize;             //是否变成守护进程标志
private:
	void display_usage();

	void display_para();

	int conf_readline(int fd, char *buff, int len);

	int Para_CmdParse(int argc, char *argv[]);

	void Para_FileParse(std::string file);

};

#endif
