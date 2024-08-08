 #include <ros/ros.h>
#include "shutils.h"
#include "utils.h"
#include <vector>
#include <stdio.h>
#include <regex>
#include <sstream>
#include <iomanip>

const std::regex channel_ex("Channel:(\\d+)");
const std::regex rssi_ex("Signal level=([\\-0-9]+) dBm");

int main(int argc, char* argv[]){
  ros::init(argc, argv, "ap_scanner", ros::init_options::NoSigintHandler);
  ros::NodeHandle nh("~");
  std::string iface;
  std::string topic;
  double scan_period;
  mac_filter filter;
  std::string mac_filter_init;
  nh.param<std::string>("iface", iface);
  nh.param<std::string>("topic", topic, "aps");
  nh.param<std::string>("mac_filter", mac_filter_init, "*:*:*:*:*:*");
  nh.param<double>("period", scan_period, 20.0);

  filter = mac_filter_str(mac_filter_init);
  
  char topic2[64];
  sprintf(topic2,"%s_2",topic.c_str());
  char topic5[64];
  sprintf(topic5,"%s_5",topic.c_str());

  char scan_cmd[128];
  sprintf(scan_cmd,"sudo iwlist %s scanning | egrep --color=never 'Address|Signal level|Channel:' 2>&1",iface.c_str());
  
  while(ros::ok()){
	std::string s = sh_exec_block(scan_cmd);
	ROS_INFO("Detected APs:");
	size_t s_start = s.find("Cell",0);
	size_t s_end = s_start;
	while((s_end=s.find("Cell", s_start+1)) != std::string::npos){
	  std::string entry = s.substr(s_start, s_end-s_start);
	  std::smatch mac_match;
	  std::smatch chan_match;
	  std::smatch rssi_match;
	  
	  bool pass_filt = true;
	  int channel, rssi;
	  if(std::regex_search(entry,chan_match,channel_ex)){
		channel = std::stoi(chan_match[1].str());
	  }
	  else pass_filt = false;
	  
	  if(std::regex_search(entry,rssi_match, rssi_ex)){
		rssi = std::stoi(rssi_match[1].str());
	  }
	  else pass_filt = false;

	  s_start = s_end;
	  if(!pass_filt) continue;
	  
	  ROS_INFO("chan %d\trssi %d",channel,rssi);
	}

    ros::Duration(scan_period).sleep();
    ROS_INFO("time is %f", ros::Time::now().toSec());
  }
}

