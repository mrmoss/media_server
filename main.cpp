//Threaded Web Server Source
//	Created By:		Mike Moss
//	Modified On:	01/31/2014

//Unix Only
//Install vlc and xdotools for this to work.
//	VLC is the only media player you need.
//	"xdotools" is a command line keyboard emulator.

//Algorithm Header
#include <algorithm>

//File Utility Header
#include "msl/file_util.hpp"

//IO Stream Header
#include <iostream>

//JSON Header
#include "msl/json.hpp"

//Socket Header
#include "msl/socket.hpp"

//Socket Utility Header
#include "msl/socket_util.hpp"

//Standard Library Header
#include <stdlib.h>

//String Header
#include <string>

//String Stream Header
#include <sstream>

//Vector Header
#include <vector>

//Web Server Threaded Header
#include "msl/webserver_threaded.hpp"

//Global Media Root Variable
std::string media_root=".";

//Our Service Client Function Declaration
bool service_client(msl::socket& client,const std::string& message);

void show_help();

class directory_list;

std::string add_escape_characters(std::string str);

directory_list find_files(const std::string& root=".");

msl::json listing(const std::string& root=".",std::string tabs="");

bool find_in_listing(const std::string& root,const std::string& title);

//Main
int main(int argc,char* argv[])
{
	//Create Port
	std::string server_port="2310";

	//No Arguments, Show Help
	if(argc==1)
			show_help();

	//Get Command Line Root Directory
	if(argc>1)
	{
		media_root=argv[1];

		if(media_root=="--help")
			show_help();
	}

	//Get Command Line Port
	if(argc>2)
	{
		server_port=argv[2];

		if(media_root=="--help")
			show_help();
	}

	//Create Server
	msl::webserver_threaded server("0.0.0.0:"+server_port,service_client);
	server.close();
	server.setup();

	//Check Server
	if(server.good())
	{
		std::cout<<"=)"<<std::endl;
	}
	else
	{
		std::cout<<"=("<<std::endl;
		exit(0);
	}

	//Be a server...forever...
	while(true)
	{
		//Update Server
		server.update();

		//Give OS a Break
		usleep(0);
	}

	//Call Me Plz T_T
	return 0;
}

//Our Service Client Function Definition
bool service_client(msl::socket& client,const std::string& message)
{
	//Create Parser
	std::istringstream istr(message);

	//Parse the Request
	std::string request;
	istr>>request;
	istr>>request;

	//Translate Request
	request=msl::http_to_ascii(request);

	//Check For Listing
	if(request=="/list")
	{
		//std::cout<<"list request"<<std::endl;

		//Send Listing
		std::string response=msl::http_pack_string(listing(media_root).str(),"application/json");
		//std::cout<<response<<std::endl;
		client.write(response.c_str(),response.size());

		//Kill Client
		client.close();

		//Return True (We serviced the client)
		return true;
	}

	//Check For Open
	else if(msl::starts_with(request,"/open="))
	{
		//std::cout<<"open request"<<std::endl;

		//Kill Client
		client.close();

		//Extract JSON Object
		msl::json videos(request.substr(6,request.size()-6));

		//If Videos Received
		if(msl::to_int(videos.get("size"))>0)
		{
			//Command Strings
			std::string command="vlc-wrapper --one-instance -f";
			std::string command_arguments="";

			//Go Through Received Titles to Open
			for(int ii=0;ii<msl::to_int(videos.get("size"));++ii)
			{
				//Get Title Name
				std::string title=videos.get(msl::to_string(ii));

				//If Title Exists, Add it to Play List
				if(find_in_listing(media_root,title))
				{
					title=add_escape_characters(title);
					command_arguments+=" "+title;
				}
			}

			//If There are Titles to Open
			if(command_arguments.size()>0)
			{
				//Build Commands
				command+=command_arguments;
				command+=" > run.log 2> error.log";

				//Execute Command
				int ignore=system(command.c_str());
			}
		}

		//Return True (We serviced the client)
		return true;
	}

	//Check For Play/Pause
	else if(request=="/play_pause")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("xdotool key space");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Quit
	else if(request=="/quit")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("pkill vlc");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Aspect Ration
	else if(request=="/aspect_ratio")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("xdotool key a");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Fast Forward
	else if(request=="/fast_forward")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("xdotool key shift+Right");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Fast Backward
	else if(request=="/fast_backward")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("xdotool key shift+Left");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Skip Forward
	else if(request=="/skip_forward")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("xdotool key ctrl+Right");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Skip Backward
	else if(request=="/skip_backward")
	{
		//Kill Client
		client.close();

		//Execute Command
		int ignore=system("xdotool key ctrl+Left");

		//Return True (We serviced the client)
		return true;
	}

	//Default Return False (We did not service the client)
	return false;
}

void show_help()
{
	std::cout<<"Media Server"<<std::endl;
	std::cout<<"\tUsage: \"./media_server MEDIA_ROOT PORT_NUMBER\""<<std::endl;
	std::cout<<"\tFiles outside of the MEDIA_ROOT folder are not accessible or viewable."<<std::endl;
	std::cout<<"\tOnly .avi, .m4v, .mkv, and .mp4 files can be played."<<std::endl;
	std::cout<<"\t\tThis can be changed by editing line 367 in the source (main.cpp)."<<std::endl;
	std::cout<<"\tDefault port is 2310."<<std::endl;
	exit(0);
}

class directory_list
{
	public:
		directory_list(const std::vector<std::string>& FOLDERS,const std::vector<std::string>& FILES):folders(FOLDERS),files(FILES)
		{}

		std::vector<std::string> folders;
		std::vector<std::string> files;
};

std::string add_escape_characters(std::string str)
{
	for(unsigned int ii=0;ii<str.size();++ii)
	{
		if(str[ii]==' '||str[ii]=='('||str[ii]==')'||str[ii]=='\''||
			str[ii]=='&'||str[ii]=='`'||str[ii]=='\"')
		{
			str.insert(ii,"\\");
			++ii;
		}
	}

	return str;
}

directory_list find_files(const std::string& root)
{
	std::string command_d="find \""+root+"\" -maxdepth 1 -type d > network_test_output_d";
	std::string command_f="find \""+root+"\" -maxdepth 1 -type f > network_test_output_f";

	int ignore_d=system(command_d.c_str());
	int ignore_f=system(command_f.c_str());

	std::string output_d;
	std::string output_f;

	msl::file_to_string("network_test_output_d",output_d);
	msl::file_to_string("network_test_output_f",output_f);

	msl::remove_file("network_test_output_d");
	msl::remove_file("network_test_output_f");

	std::vector<std::string> vector_d;
	std::vector<std::string> vector_f;
	std::string temp="";

	for(unsigned int ii=0;ii<output_d.size();++ii)
	{
		if(output_d[ii]=='\n'||ii==output_d.size()-1)
		{
			if(temp!=root)
				vector_d.push_back(temp);

			temp="";
		}
		else
		{
			temp+=output_d[ii];
		}
	}

	for(unsigned int ii=0;ii<output_f.size();++ii)
	{
		if(output_f[ii]=='\n'||ii==output_f.size()-1)
		{
			if(msl::ends_with(temp,".avi")||msl::ends_with(temp,".mp4")||msl::ends_with(temp,".mkv")||msl::ends_with(temp,".m4v"))
				vector_f.push_back(temp);

			temp="";
		}
		else
		{
			temp+=output_f[ii];
		}
	}

	return directory_list(vector_d,vector_f);
}

msl::json listing(const std::string& root,std::string tabs)
{
	msl::json ret;
	ret.set("name",root);
	tabs+="\t";

	directory_list current_list(find_files(root));

	std::sort(current_list.folders.begin(),current_list.folders.end());
	std::sort(current_list.files.begin(),current_list.files.end());

	msl::json files;
	files.set("size",current_list.files.size());

	for(unsigned int ii=0;ii<current_list.files.size();++ii)
		files.set(msl::to_string(ii),current_list.files[ii]);

	ret.set("files",files.str());

	msl::json directories;
	directories.set("size",current_list.folders.size());

	for(unsigned int ii=0;ii<current_list.folders.size();++ii)
		directories.set(msl::to_string(ii),listing(current_list.folders[ii],tabs).str());

	ret.set("directories",directories.str());

	return ret;
}

bool find_in_listing(const std::string& root,const std::string& title)
{
	directory_list current_list(find_files(root));

	for(unsigned int ii=0;ii<current_list.files.size();++ii)
		if(current_list.files[ii]==title)
			return true;

	for(unsigned int ii=0;ii<current_list.folders.size();++ii)
		if(find_in_listing(current_list.folders[ii],title))
			return true;

	return false;
}
