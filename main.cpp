#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <csignal>
int main(int argc,char**argv){
	std::signal(SIGPIPE,SIG_IGN);
	int port=argc>1?std::stoi(argv[1]):8080;
	int s=socket(AF_INET,SOCK_STREAM,0);
	int opt=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=INADDR_ANY;addr.sin_port=htons(port);
	if(bind(s,(sockaddr*)&addr,sizeof(addr))<0||listen(s,128)<0){perror("bind/listen");return 1;}
	std::cout<<"listening "<<port<<"\n";
	for(;;){
		int c=accept(s,0,0);
		if(c<0)continue;
		char buf[4096];ssize_t n=read(c,buf,sizeof(buf));
		std::string req(buf,n);
		std::string path="index.html";
		size_t p1=req.find("GET ");
		if(p1!=std::string::npos){
			size_t p2=req.find(' ',p1+4);
			if(p2!=std::string::npos){
				std::string tmp=req.substr(p1+4,p2-(p1+4));
				if(tmp!="/")path=tmp.substr(1);
			}
		}
		std::ifstream f(path);
		std::stringstream ss;ss<<f.rdbuf();
		std::string body=ss.str();
		std::string type="text/plain";
		if(path.size()>=5&&path.substr(path.size()-5)==".html")type="text/html";
		else if(path.size()>=4&&path.substr(path.size()-4)==".css")type="text/css";
		else if(path.size()>=3&&path.substr(path.size()-3)==".js")type="application/javascript";
		std::string res="HTTP/1.1 200 OK\r\nContent-Type: "+type+"\r\nContent-Length: "+std::to_string(body.size())+"\r\nConnection: close\r\n\r\n"+body;
		write(c,res.data(),res.size());
		close(c);
	}
}