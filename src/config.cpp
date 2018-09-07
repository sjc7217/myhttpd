#include "header.h"

static char *l_opt_arg;
static std::string shortopts = "a:c:d:f:hi:o:l:m:t:s";

static struct option longopts[] = {
		{"CGIRoot",      required_argument, nullptr, 'c'},
		{"DefaultFile",  required_argument, nullptr, 'd'},
		{"DocumentRoot", required_argument, nullptr, 'o'},
		{"ConfigFile",   required_argument, nullptr, 'f'},
		{"ListenIP",     required_argument, nullptr, 'a'},
		{"ListenPort",   required_argument, nullptr, 'l'},
		{"MaxWorker",    required_argument, nullptr, 'm'},
		{"TimeOut",      required_argument, nullptr, 't'},
		{"InitConPool",  required_argument, nullptr, 'i'},
		{"Help",         no_argument,       nullptr, 'h'},
		{"Daemonize",    no_argument,       nullptr, 's'},
		{0, 0,                              0,       0},
};

Config::Config() {
	CGIRoot = "./cgi/";
	DefaultFile = "index.html";
	DocumentRoot = "./htdocs/";
	ConfigFile = "./myhttpd.conf";
	ListenIP = "0.0.0.0";
	ListenPort = 8000;
	MaxWorker = 4;
	TimeOut = 3;
	InitConPool = 200;
    Daemonize = false;
//	PluginList.push_back("plugin/plugin_static/plugin_static.so");
//	PluginList.push_back("plugin/plugin_cgi/plugin_cgi.so");
}

Config::~Config() {

}

void Config::display_usage() {
	std::cout << "./slighttpd [option]..." << std::endl;
    std::cout << "  -l|--ListenPort<number>     Default: 8000" << std::endl;
    std::cout << "  -m|--MaxWorker<number>      Default: 4" << std::endl;
    std::cout << "  -i|--InitConPool<number>    Default: 200" << std::endl;
    std::cout << "  -a|--ListenIP<address>      Default: 0.0.0.0" << std::endl;
    std::cout << "  -o|--DocumentRoot<path>     Default: ./htdocs/" << std::endl;
    std::cout << "  -c|--CGIRoot<path>          Default: ./cgi/" << std::endl;
    std::cout << "  -d|--DefaultFile<filename>  Default: index.html" << std::endl;
    std::cout << "  -t|--TimeOut<seconds>       Default: 3" << std::endl;
    std::cout << "  -f|--ConfigFile<filename>   Default: ./slighttpd.conf" << std::endl;
    std::cout << "  -s|--Daemonize              Default: Daemonize Disabled" << std::endl;
}

void Config::display_para() {
	std::cout << "slighttpd - Simple Http Server Demo" << std::endl;
	std::cout << std::endl;
	std::cout << "-ListenIP:	" << ListenIP << std::endl;
	std::cout << "-ListenPort:	" << ListenPort << std::endl;
	std::cout << "-MaxWorker:	" << MaxWorker << std::endl;
	std::cout << "-InitConPool:	" << InitConPool << std::endl;
	//std::cout << "-DocumentRoot:	" << DocumentRoot << std::endl;
	//std::cout << "-CGIRoot:	" << CGIRoot << std::endl;
	//std::cout << "-DefaultFile:	" << DefaultFile << std::endl;
	//std::cout << "-TimeOut:	" << TimeOut << std::endl;
	std::cout << "-ConfigFile:	" << ConfigFile << std::endl;
    std::cout << "-Daemonize:	" << (Daemonize ? "Enables" : "Disabled") << std::endl;
	for (int i = 0; i < PluginList.size(); ++i)
		std::cout << "-Plugin:	" << PluginList[i] << std::endl;
}

int
Config::conf_readline(int fd, char *buff, int len)                              //每次读入配置文件的一行，碰到文件末尾或者'\r','\n'换行重新获取
{
	ssize_t n = -1;
	int i = 0;
	int begin = 0;

	/*清缓冲区*/
	memset(buff, 0, len);
	for (i = 0; i < len; begin ? i++ : i)/*当开头部分不为'\r'或者'\n'时i计数*/
	{
		n = read(fd, buff + i, 1);/*读一个字符*/
		if (n == 0)/*文件末尾*/{
			*(buff + i) = '\0';
			break;
		} else if (*(buff + i) == '\r' || *(buff + i) == '\n') {/*回车换行*/
			if (begin) {/*为一行*/
				*(buff + i) = '\0';
				break;
			}
		} else {
			begin = 1;             //表示读到了一行有效数字
		}
	}

	return i;                      //返回读到的配置行的长度
}


int Config::Para_CmdParse(int argc, char *argv[]) { //处理命令行参数，使用命令行参数更新程序配置

	int c;
	ssize_t len;
	int value;

	/*遍历输入参数,设置配置参数*/
	while ((c = getopt_long(argc, argv, shortopts.c_str(), longopts, nullptr)) != -1) {
		switch (c) {
			case 'a':    /*监听地址*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					ListenIP = l_opt_arg;
				}

				break;
			case 'c':    /*CGI根路径*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					CGIRoot = l_opt_arg;
				}

				break;
			case 'd':    /*默认文件名称*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					DefaultFile = l_opt_arg;
				}

				break;
			case 'f':    /*配置文件名称和路径*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					ConfigFile = l_opt_arg;
				}

				break;
			case 'o':       /*根文件路径*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					DocumentRoot = l_opt_arg;
				}

				break;
			case 'i':    /*连接池大小*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, nullptr, 10);
					if (value != LONG_MAX && value != LONG_MIN)
						InitConPool = value;
				}

				break;
			case 'l':    /*侦听端口*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, nullptr, 10);
					if (value != LONG_MAX && value != LONG_MIN)
						ListenPort = value;
				}

				break;
			case 'm': /*最大子进程数量*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, nullptr, 10);
					if (value != LONG_MAX && value != LONG_MIN)
						MaxWorker = value;
				}

				break;
			case 't':    /*超时时间*/
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					std::cout << "TIMEOUT" << std::endl;
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, nullptr, 10);
					if (value != LONG_MAX && value != LONG_MIN)
						TimeOut = value;
				}

                break;
            case 's':    /*是否开启守护进程模式*/
                Daemonize = true;
				break;
			case '?':/*错误参数*/
				std::cout << "Invalid para" << std::endl;
				return -1;
			case 'h':    /*帮助*/
				display_usage();
				return -1;
		}
	}
	return 0;
}

/*
*  解析配置文件参数
*/
void Config::Para_FileParse(std::string file) {                //根据配置文件更新程序变量
#define LINELENGTH 256
	char line[LINELENGTH];
	char *name = nullptr, *value = nullptr;
	int fd = -1;
	int n = 0;

	fd = open(file.c_str(), O_RDONLY);
	if (fd == -1) goto EXITPara_FileParse;

	//插件列表以配置文件为准
	PluginList.clear();
	/*
	*命令格式如下:
	*[#注释|[空格]关键字[空格]=[空格]value]
	*/
	while ((n = conf_readline(fd, line, LINELENGTH)) != 0) {                    //一行一行处理配置文件
		char *pos = line;
		/* 跳过一行开头部分的空格 */
		while (isspace(*pos)) {
			pos++;
		}
		/*注释?*/
		if (*pos == '#') {                                                        //注释部分直接跳过
			continue;
		}

		/*关键字开始部分*/
		name = pos;
		/*关键字的末尾*/
		while (!isspace(*pos) && *pos != '=')                                    //直接找到配置选项的末尾
		{
			pos++;
		}
		*pos = '\0';/*生成关键字字符串*/                                            //配置选项字符串
		pos++;

		/*value部分前面空格*/
		while (isspace(*pos) || (*pos) == '=') {
			pos++;
		}
		/*value开始*/
		value = pos;
		/*到结束*/
		while (!isspace(*pos) && *pos != '\r' && *pos != '\n' && *pos != '\0')        //配置值获取
		{
			pos++;
		}
		*pos = '\0';/*生成值的字符串*/
		//std::cout<< name <<std::endl;
		/*根据关键字部分，获得value部分的值*/
		int ivalue;
		/*"CGIRoot","DefaultFile","DocumentRoot","ListenIP","ListenPort","MaxClient","InitConPool","TimeOut"*/
		if (0 == strncmp("CGIRoot", name, 7)) {/*CGIRoot部分*/                     //一个个判断是否是配置文件值，是的话更新配置
			CGIRoot = value;
		} else if (0 == strncmp("DefaultFile", name, 11)) {/*DefaultFile部分*/
			DefaultFile = value;
		} else if (0 == strncmp("DocumentRoot", name, 12)) {/*DocumentRoot部分*/
			DocumentRoot = value;
		} else if (0 == strncmp("Plugin", name, 6)) {/*Plugin部分*/
			PluginList.push_back(value);
		} else if (0 == strncmp("ListenIP", name, 8)) {/*ListenIP部分*/
			ListenIP = value;
		} else if (0 == strncmp("ListenPort", name, 10)) {/*ListenPort部分*/
			ivalue = strtol(value, nullptr, 10);
			ListenPort = ivalue;
		} else if (0 == strncmp("MaxWorker", name, 9)) {/*MaxClient部分*/
			ivalue = strtol(value, nullptr, 10);
			MaxWorker = ivalue;
		} else if (0 == strncmp("InitConPool", name, 11)) {/*InitConPool部分*/
			ivalue = strtol(value, nullptr, 10);
			InitConPool = ivalue;
		} else if (0 == strncmp("TimeOut", name, 7)) {/*TimeOut部分*/
			ivalue = strtol(value, nullptr, 10);
			TimeOut = ivalue;
        } else if (0 == strncmp("Daemonize", name, 9)) {/*Daemonize部分*/
            ivalue = strtol(value, nullptr, 10);
            Daemonize = ivalue ? true : false;
        }
	}
	close(fd);

	EXITPara_FileParse:
	return;
}


int Config::InitPara(int argc, char *argv[]) {
	/*解析命令行输入参数*/
	if (0 != Para_CmdParse(argc, argv))
		return -1;
	/*解析配置文件配置参数*/
	if (ConfigFile.size())
		Para_FileParse(ConfigFile);

	display_para();

	/*返回配置参数*/
	return 0;
}

