#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <math.h>

using namespace std;

	/*
	This simulation of IPv4 is based on the RIPv2
	IPv6 simulation is based on the RIPng
	Author:
	Wu Zhenyu 1120170664 
	Li Huiyi 1120172766
	*/
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
#define request 1
#define reply 2
#define AF_INET 10

typedef struct table{
	uint Metric;
	uchar NextHop[4];
	uchar SubnetMask[4];
	uchar IPaddress[4];
	
	struct table *next;
}RIPtable;
typedef struct protocol{
	ushort RouterTag;
	ushort AFI;
	ushort Unused;
	uchar Version;
	uchar Command;
	RIPtable *next;
}RIPheader;
typedef struct port{
	int subnetMask[4];
	int netid[4];
	int portid[4]; 
	int portNum;
	struct port *next;
}Port;
typedef struct Routertablelist{
	int dest_netid[4];
	int dest_netMask[4];
	int next_hop[4];
	int num_of_hops;
	Routertablelist *next;
}Routertable;
typedef struct data{
	string message;
	int dest_ip[4];
}Datapack;
class Router{//define router class
	public:
		string name; //the name of the router
		//string buffer;
		Datapack datapack;//the datapack to transfer
		Router();
		Port *port;
		Routertable *list;//Router table, store in linked list
		void printRoutingTable();
		void addNet(int subnetMask[],int netid[],int portid[]);
		void addRoutertable(int dest_netid[],int dest_netMask[],int next_hop[],int num_of_hops);
		void generateProtocol();
		~Router();		
};
void func(void *pBuffer, int nSize){
	unsigned char *pTemp = (unsigned char*)pBuffer;
	int i,j,nResult;
	for (i=nSize-1;i>=0;i--){
		for (j=7;j>=0;j--){
			nResult = pTemp[i] & (int)pow(2,j);
			nResult = (0 != nResult);
			cout<<nResult;
		}
		cout<<" ";
		if(i%4 == 0) cout<<endl;
	}
}
void Router::generateProtocol(){
	RIPheader *newheader = new RIPheader;
	newheader->Command = reply;
	newheader->AFI = AF_INET;
	newheader->RouterTag = 0;
	newheader->Unused = 0;
	newheader->Version = 2;//use RIPv2 protocol;
	Routertable *search = this->list->next;
	RIPtable *table = new RIPtable;
	newheader->next = table ;
	/*if(search != NULL){
		table = new RIPtable;
		newheader->next = table;
	}*/
	table->next = NULL;
	while(search != NULL){
		//table->next = new RIPtable;
		//table = table->next;
		//table->next = NULL;
		//if(table->next != NULL)
		for(int i=0;i<4;i++){
			table->IPaddress[i] = uchar(search->dest_netid[3-i]);
			table->SubnetMask[i] = uchar(search->dest_netMask[3-i]);
			table->NextHop[i] = uchar(search->next_hop[3-i]);
		}
		table->Metric = uint(search->num_of_hops);
		//RIPtable *newtable = new RIPtable 
		table->next = new RIPtable;
		table = table->next;
		table->next = NULL;
		search = search->next;
	}
	cout<<"Print the RIP datagram"<<endl;
	//cout<<hex<<newheader->Command<<endl;
	func(newheader,sizeof(RIPheader)-8);
	cout<<endl;
	table = newheader->next;
	while(table->next != NULL){
		func(table,sizeof(RIPtable)-8);
		table = table->next;
		cout<<endl;
	}
}
bool compare2net(int add1[],int add2[]){
	bool flag = 1;
	for(int i=0;i<4;i++){
		if(add1[i] != add2[i]){
			flag = 0;
			break;
		}
	}
	return flag;
}
void printIP(int ip[]){
	cout<<ip[0]<<"."<<ip[1]<<"."<<ip[2]<<"."<<ip[3];
}
Router::Router(){
	this->name = "nameless";
	this->port = new Port;
	this->port->next = NULL;
	this->port->portNum = 0;//the initial port num
	for(int i=0;i<4;i++){
		this->port->portid[i] = 0;
		this->port->subnetMask[i] = 0;
		this->port->netid[i] = 0;
	}
	this->list = new Routertable;
	this->list->next = NULL;
	for(int i=0;i<4;i++){
		this->list->dest_netid[i] = 0;
		this->list->dest_netMask[i] = 0;
		this->list->next_hop[i] = 0;
	}
	this->list->num_of_hops = 16;
}
Router::~Router(){
	delete[] port;
	delete[] list;
}
void Router::addRoutertable(int dest_netid[],int dest_netMask[],int next_hop[],int num_of_hops){//the function that add one term to the routertable
	Routertable *newList = new Routertable;
	for(int i=0;i<4;i++){
		newList->dest_netid[i] = dest_netid[i];
		newList->dest_netMask[i] = dest_netMask[i];
		newList->next_hop[i] = next_hop[i];
		newList->num_of_hops = num_of_hops;
	}
	newList->next = NULL;
	Routertable *find = this->list;
	while(find->next != NULL){
		find = find->next;
	}
	find->next = newList;
}
void Router::addNet(int subnetMask[],int netid[],int portid[]){
	Port *newNode = new Port;
	for(int i=0;i<4;i++){
		newNode->subnetMask[i] = subnetMask[i];
		newNode->netid[i] = netid[i];
		newNode->portid[i] = portid[i];
	}
	newNode->next = NULL;
	Port *find = this->port;
	while(find->next != NULL){
		find = find->next; 
	} 
	newNode->portNum = find->portNum + 1;
	find->next = newNode;
	int next_hopip[4] = {0,0,0,0};
	this->addRoutertable(netid,subnetMask,next_hopip,0);
}
//connect one net to the router, automatically refresh the router table
void Router::printRoutingTable(){
	cout<<"================ The routing table of "<<this->name<<" ================"<<endl;
	Routertable *index = this->list;
	index = index->next;
	int count = 1;
	while(index != NULL){
		cout<<count<<". Destination net ID: ";
		printIP(index->dest_netid);
		cout<<" via ";
		printIP(index->next_hop);
		cout<<setw(10)<<" num of hops is "<<index->num_of_hops<<endl;
		count++;
		index = index->next;
	}
}
//print the router table
string intToStr(int x){
    stringstream convert_to_string;
    convert_to_string << x;
    return convert_to_string.str();
}
bool compareNet(Routertable *list1,Routertable *list2){
	bool flag = 1;
	for(int i=0;i<4;i++){
		if((list1->dest_netMask[i] != list2->dest_netMask[i])||(list1->dest_netid[i] != list2->dest_netid[i])){
			flag = 0;
			break;
		}
	}
	return flag;
}
//compare 2 nets in the router table
bool compareNet2(Port *list1,Port *list2){
	bool flag = 1;
	for(int i=0;i<4;i++){
		if((list1->subnetMask[i] != list2->subnetMask[i])||(list1->netid[i] != list2->netid[i])){
			flag = 0;
			break;
		}
	}
	return flag;
}
//compare 2 nets in the port
Port* findlink(Router &from,Router &to){
	Port *index_from = from.port;
	index_from = index_from->next;
	Port *index_to = to.port;
	index_to = index_to->next;
	Port *index_to_copy = index_to;
	//Routertable *list_from = from.list->next;
	Routertable *list_to = to.list->next;
	//Routertable *list_from_copy = list_from;
	Routertable *list_to_copy;
	while(index_from != NULL){
		index_to_copy = index_to;
		while(index_to_copy !=NULL){
			if(compareNet2(index_from,index_to_copy)){
			// 	list_to_copy = list_to;
			//	while(list_to_copy != NULL){
			//		if(compare2net(list_to_copy->next_hop,index_from->portid)){
			//			if(list_to_copy->num_of_hops != 16){
							return index_from;
			//			}
			//		}
			//		list_to_copy = list_to_copy->next;
			//	}	
			}
			index_to_copy = index_to_copy->next;
		}
		index_from = index_from->next;
	}
	return NULL;
}
void destroyLink(Router &router1,Router &router2){
	int *portid1 = findlink(router1,router2)->portid;
	int *portid2 = findlink(router2,router1)->portid;
	Routertable *router1table = router1.list->next;
	Routertable *router2table = router2.list->next;
	while(router1table != NULL){
		if(compare2net(portid2,router1table->next_hop)){
			router1table->num_of_hops = 16; //set port unreachable;
		}
		router1table = router1table->next;
	}
	while(router2table != NULL){
		if(compare2net(portid1,router2table->next_hop)){
			router2table->num_of_hops = 16;
		}
		router2table = router2table->next;
	}
	Port *pre,*cur,*aim;
	aim = findlink(router1,router2);
	cur = router1.port->next;
	pre = router1.port;
	while(cur != NULL){
		if(cur == aim){
			pre->next = cur->next;
			delete cur;
			break;
		}
		cur = cur->next;
		pre = pre->next;
	}
	aim = findlink(router2,router1);
	cur = router2.port->next;
	pre = router2.port;
	while(cur != NULL){
		if(cur == aim){
			pre->next = cur->next;
			delete cur;
			break;
		}
		cur = cur->next;
		pre = pre->next;
	}
}
//find a connection between to router, the return value is the port of the former one
bool searchlist(Routertable *list1,Routertable *list2){
	Routertable *find = list2;
	find = find->next;
	bool flag = 0;
	while(find != NULL){
		if(compareNet(list1,find)){
			flag = 1;
			break;
		}
		find = find->next;
	}
	return flag;
}
//see whether the first term is in the nest router table
void RIP(Router &from,Router &to){
	cout<<endl;
	cout<<"======== RIP from Router: " << from.name << " to Router: " << to.name <<"========="<< endl;    
    from.printRoutingTable();
    to.printRoutingTable();
    
    Routertable *index_from = from.list;
    index_from = index_from->next;
    Routertable *index_to = to.list;
    index_to = index_to->next;
    Routertable *index_to_copy = index_to;
    while(index_from != NULL){
    	index_to_copy = index_to;
    	while(index_to_copy != NULL){
    		int count = 0;
    		if(compareNet(index_from,index_to_copy)){
    			count++;
    			if(index_from->num_of_hops == 16){
    				index_to_copy->num_of_hops = index_from->num_of_hops;
    				for(int i=0;i<4;i++){
    					index_to_copy->next_hop[i] = findlink(from,to)->portid[i]; 
					}
					break;
				}//destination unreachable
    			if(index_from->num_of_hops + 1 < index_to_copy->num_of_hops){
    				index_to_copy->num_of_hops = index_from->num_of_hops + 1;
    				for(int i=0;i<4;i++){
    					index_to_copy->next_hop[i] = findlink(from,to)->portid[i]; 
					}
					break;
				}
			}
			index_to_copy = index_to_copy->next;
		}
		if(!searchlist(index_from,to.list)){
			to.addRoutertable(index_from->dest_netid,index_from->dest_netMask,findlink(from,to)->portid,index_from->num_of_hops+1);
		//	to.addRoutertable(index_from->dest_netid,index_from->dest_netMask,from.port->portid,index_from->num_of_hops+1);
		}
		index_from = index_from->next; 
	}
    
    cout<<endl;
    cout<< "------ Resulting routing table ------"<<endl;
    to.printRoutingTable();
    cout<< "================================================="<<endl;
}
//RIP protocal main function
void RIPall(Router routergroup[],int numOfRouter){
	for(int i=1;i<=numOfRouter;i++){
		for(int j=1;j<=numOfRouter;j++){
			if(j == i) continue;
			if(findlink(routergroup[i],routergroup[j])!=NULL){
				
				RIP(routergroup[i],routergroup[j]);
			}
		}
	}
}
//all routers RIP the nearby router 
void rename(Router &router,string newname){
	router.name = newname;
}
Router &dataTransfer(Router &start,Router routergroup[],int routernum){
	bool searchflag = 0;
	Routertable *search = start.list;
	search = search->next;
	int netid[4];
	int direct[4] = {0,0,0,0};
	while(search != NULL){
		for(int i=0;i<4;i++){
			netid[i] = start.datapack.dest_ip[i] & search->dest_netMask[i];
		}
		if(compare2net(netid,search->dest_netid)){
			if(search->num_of_hops == 16){
				cout<<"Destination unreachable!"<<endl;
				return start;
			}
			searchflag = 1;
			break;
		}
		search = search->next;
	}//find destination netid in the router table;
	if(!searchflag){
		cout<<"Destination unreachable!"<<endl;
		return start;
	}
	
	if(compare2net(search->next_hop,direct)){
		cout<<"Already reach destination at router "<<start.name<<endl;
		return start;
	}
	//search->next_hop
	searchflag = 0;
	Port *currentPort;
	for(int i=1;i<=routernum;i++){
		currentPort = routergroup[i].port;
		currentPort = currentPort->next;
		while(currentPort != NULL){
			if(compare2net(search->next_hop,currentPort->portid)){
				routergroup[i].datapack.message = start.datapack.message;
				cout<<" via "<<routergroup[i].name<<". ";
				for(int j=0;j<4;j++){
					routergroup[i].datapack.dest_ip[j] = start.datapack.dest_ip[j];
				}
				return dataTransfer(routergroup[i],routergroup,routernum);
			}
			currentPort = currentPort->next;
		}
	}
}
int main(){
	Router router[6];
    int N1[4] = {192,168,1,0};
    int N2[4] = {192,168,2,0};
    int N3[4] = {192,168,3,0};
    int N4[4] = {192,168,4,0};
    int N5[4] = {192,168,5,0};
    int port12[4] = {192,168,1,1};
    int port14[4] = {192,168,2,1};
    int port21[4] = {192,168,1,254};
    int port41[4] = {192,168,2,254};
    int port23[4] = {192,168,5,1};
    int port45[4] = {192,168,3,1};
    int port32[4] = {192,168,5,254};
    int port54[5] = {192,168,3,254};
    int port35[4] = {192,168,4,1};
    int port53[4] = {192,168,4,254};
    int subnetMask[4] = {255,255,255,0};
    rename(router[1],"router1");
	rename(router[2],"router2");
	rename(router[3],"router3");
	rename(router[4],"router4");
	rename(router[5],"router5"); 
    router[1].addNet(subnetMask, N1, port12);
    //router[1].addNet(subnetMask, N2, port14);
    router[2].addNet(subnetMask, N1, port21);
    router[2].addNet(subnetMask, N5, port23);
    router[3].addNet(subnetMask, N5, port32);
    router[3].addNet(subnetMask, N4, port35);
    //router[4].addNet(subnetMask, N2, port41);
    router[4].addNet(subnetMask, N3, port45);
    router[5].addNet(subnetMask, N3, port54);
    router[5].addNet(subnetMask, N4, port53);
    //the demostrasion RIP process,a ring network with number of routers 5.
    
    Router newrouter[100];
	int routerNum = 0;
	int num1,num2;
    string routername;
	int choice;
	int portip[4],netip[4],subnet[4];
	int times;
    while(1){
    	cout<<"Please choose the function tou want:"<<endl;
    	cout<<"1. Create a new router."<<endl;
    	cout<<"2. Add link to a router."<<endl;
    	cout<<"3. Print a routertable as well as the RIP datagram."<<endl;
    	cout<<"4. RIP from one router to another."<<endl;
    	cout<<"5. All routers RIP."<<endl;
    	cout<<"6. Display a demostration."<<endl;
    	cout<<"7. Send a message."<<endl;
    	cin>>choice;
    	switch(choice){
    		case 1:
    			routerNum++;
    			cout<<"--Please enter the name of router: ";
    			cin>>routername;
    			rename(newrouter[routerNum],routername);
    			cout<<"--Router was created successfully."<<endl;
    			cout<<"--The total num of the routers is "<<routerNum<<endl;
    			cout<<endl;
    			break;
    		case 2:
    			cout<<"--Please enter the num of the router: ";
    			cin>>num1;
    			cout<<endl;
    			cout<<"--The router num is "<<num1<<", the corresponding name of the router is "<<newrouter[num1].name<<endl;
    			cout<<"--Please enter the port IP address, net address and the subnet mask sequencely: "<<endl;
    		
    			for(int i=0;i<4;i++){
    				cin>>portip[i];
				}
				for(int i=0;i<4;i++){
    				cin>>netip[i];
				}
				for(int i=0;i<4;i++){
    				cin>>subnet[i];
				}
				newrouter[num1].addNet(subnet,netip,portip);
				cout<<"--A new link was added to the router successfully."<<endl;
				cout<<endl;
				break;
			case 3:
				cout<<"--Please enter the num of the router: ";
				cin>>num1;
				cout<<endl;
				newrouter[num1].printRoutingTable();
				cout<<"--The RIPv2 datagram is shown as follow: "<<endl;
				newrouter[num1].generateProtocol();
				cout<<endl;
				break;
			case 4:
				cout<<"--Please enter the first router num: ";
				cin>>num1;
				cout<<endl;
				cout<<"--Please enter the next router num: ";
				cin>>num2;
				RIP(newrouter[num1],newrouter[num2]);
				cout<<endl;
				break;
			case 5:
				cout<<"--Please enter the times of RIP: ";
				cin>>times;
				cout<<endl;
				for(int i=0;i<times;i++){
					RIPall(newrouter,routerNum);
				}
				cout<<endl;
				break;
			case 6:
				for(int i=0;i<10;i++){
					RIPall(router,5);
				}
				cout<<endl;
				break;
			case 7:
				cout<<"--Please choose the source router: ";
				cin>>num1;
				cout<<endl;
    			cout<<"--The router num is "<<num1<<", the corresponding name of the router is: "<<newrouter[num1].name<<endl;
    			cout<<"--Please input the message and the destination id: "<<endl;
    			cin>>routername;
    			for(int i=0;i<4;i++){
    				cin>>portip[i];
				}
				newrouter[num1].datapack.message = routername;
				for(int i=0;i<4;i++){
    				newrouter[num1].datapack.dest_ip[i] = portip[i];
				}
				dataTransfer(newrouter[num1],newrouter,routerNum);
				break; 
		}
	} /* 
	for(int i=0;i<10;i++){
		RIPall(router,5);
	}
	router[1].datapack.message = "This is a blank message";
	router[1].datapack.dest_ip[0] = 192;
	router[1].datapack.dest_ip[1] = 168;
	router[1].datapack.dest_ip[2] = 3;
	router[1].datapack.dest_ip[3] = 7;
	
	cout<<dataTransfer(router[1],router,5).datapack.message<<endl;
	//destroyLink(router[1],router[4]);
	for(int i=0;i<10;i++){
		RIPall(router,5);
	}
	cout<<dataTransfer(router[1],router,5).datapack.message<<endl;
	/*for(int i=1;i<=5;i++){
		cout<<router[i].datapack.message<<endl;
	}*/
	//cout<<router[4].datapack.message<<endl;
	 
}

