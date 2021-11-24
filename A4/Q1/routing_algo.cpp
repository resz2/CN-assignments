#include "node.h"
#include <iostream>

using namespace std;

void printRT(vector<RoutingNode*> nd){
/*Print routing table entries*/
	for (int i = 0; i < nd.size(); i++) {
	  nd[i]->printTable();
	}
}

void routingAlgo(vector<RoutingNode*> nd){
  //Your code here
  int num = nd.size();
  int temp = num;
  while(num--)  {
    for(int i=0; i<temp; i++) {
      nd[i]->sendMsg();
    }
  }
  /*Print routing table entries after routing algo converges */
  printRT(nd);
}


void RoutingNode::recvMsg(RouteMsg *msg) {
  //your code here
  vector<RoutingEntry> senTable = msg->mytbl->tbl;

  for(int i=0; i<senTable.size(); i++)  {
    bool flag = false;
    int dup = 0;

    for(int j=0; j<mytbl.tbl.size(); j++) {
      if(mytbl.tbl[j].dstip == senTable[i].dstip) {
        // entry of sender present in receiver table
        dup = j;
        flag = true;
        break;
      }
    }

    if(flag)  {
      if(mytbl.tbl[dup].cost > senTable[i].cost + 1)  {
        mytbl.tbl[dup].cost = senTable[i].cost + 1;
      }
    }
    else  {
      RoutingEntry route;
      route.dstip = senTable[i].dstip;
      route.nexthop = msg->from;
      route.ip_interface = msg->recvip;
      if(route.dstip == route.nexthop)  {
        // next hop is the destination
        route.cost = 1;
      }
      else  {
        route.cost = 1 + senTable[i].cost;
      }
      mytbl.tbl.push_back(route);
    }
  }
}




