#include "mist.h"
#include <sys/stat.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

namespace mist {

Mist::Mist() {
  // Each init_checker is called only once when this agent is loaded
  init_checkers_.insert(new ProcInitChecker);
   
  // Each checker is called for each point
 // checkers_.insert(new IpcChecker);
  checkers_.insert(new ForkChecker(this));
//  checkers_.insert(new CloneChecker(this));
//  checkers_.insert(new ExitChecker);
//  checkers_.insert(new ChangeIdChecker);
  
 /* checkers_.insert(new FileOpenChecker);
  checkers_.insert(new LibChecker);
  checkers_.insert(new MmapChecker);
  checkers_.insert(new ChmodChecker);
  checkers_.insert(new ThreadChecker);
   */
  // Each fini_checker is called only once when this agent is unloaded
  fini_checkers_.insert(new ProcFiniChecker);
//  init_run();
   record_params();

}

Mist::~Mist() {
//  init_run();
  fini_run();

  // Clean up
  for (OneTimeCheckers::iterator i = init_checkers_.begin();
       i != init_checkers_.end(); i++) {
    delete *i;
  }

  for (Checkers::iterator i = checkers_.begin();
       i != checkers_.end(); i++) {
    delete *i;
  }

  for (OneTimeCheckers::iterator i = fini_checkers_.begin();
       i != fini_checkers_.end(); i++) {
    delete *i;
  }
}

void Mist::fork_init_run() {
  u_.ChangeTraceFile();
  init_run();
}

void Mist::record_params() {
  FILE *fp;
  if((fp=fopen("/tmp/config.txt","r")) ==NULL){ 
	perror("Couldnt open the config file");
	exit(0);
  }
  char line[100];
  fgets(line,256,fp);
  while(fgets(line, 256,fp )!=NULL)
  {
	if(strcmp(line,"post-inst-functions\n") == 0)
		break;
	std::istringstream str_line;
	str_line.str(line);
	std::string dummy_str;
	struct Inst_func new_func;
	if(std::getline(str_line,dummy_str,' ')) {
		new_func.func_name.assign(dummy_str);
	//	fprintf(stderr,"Func name : %s\t", new_func.func_name.c_str());
	}
	if(std::getline(str_line,dummy_str,' ')) {
		new_func.desc.assign(dummy_str);
	//	fprintf(stderr, "Type :%s\t", new_func.desc.c_str());
	}
	while(std::getline(str_line,dummy_str,' ')) {
		if(dummy_str.at(dummy_str.length()-1)=='\n')
			dummy_str.at(dummy_str.length()-1)='\0';	
		new_func.other_paras.push_back(dummy_str);
	//	fprintf(stderr,"Paras : %s\n", dummy_str.c_str());
	}
	pre_inst_func.push_back(new_func);
  }

  while(fgets(line, 256,fp )!=NULL)
  {     
        std::istringstream str_line;
        str_line.str(line);
        std::string dummy_str;
        struct Inst_func new_func;
        if(std::getline(str_line,dummy_str,' ')) {
                new_func.func_name.assign(dummy_str);
        //      fprintf(stderr,"Func name : %s\t", new_func.func_name.c_str());
        }
        if(std::getline(str_line,dummy_str,' ')) {
                new_func.desc.assign(dummy_str);
          //    fprintf(stderr, "Type :%s\t", new_func.desc.c_str());
        }
        while(std::getline(str_line,dummy_str,' ')) {
                if(dummy_str.at(dummy_str.length()-1)=='\n')
                        dummy_str.at(dummy_str.length()-1)='\0';
                new_func.other_paras.push_back(dummy_str);
            //  fprintf(stderr,"Paras : %s\n", dummy_str.c_str());
        }
        post_inst_func.push_back(new_func);
  }
	
     
 
  fclose(fp);
}

void Mist::init_run() {	
  // run only once for a process
  for (OneTimeCheckers::iterator i = init_checkers_.begin();
       i != init_checkers_.end(); i++) {
    (*i)->run();
  }
}

void Mist::fini_run() {
  // run fini checkers
  for (OneTimeCheckers::iterator i = fini_checkers_.begin();
       i != fini_checkers_.end(); i++) {
    (*i)->run();
  }
}

void Mist::run(SpPoint* pt, SpFunction* f) {
   static int i= 1; 
   if(i){ 	
   	init_run(); 
	i=0;
   }
  for(unsigned int i=0; i< pre_inst_func.size();i++)
  {
	struct Inst_func cur_func = pre_inst_func.at(i);
	if(f->name().compare(cur_func.func_name.c_str()) == 0 )
	{
		char buf[1024];
		snprintf(buf, 1024,"<trace type=\"%s\" time=\"%lu\">",cur_func.desc.c_str(), u_.GetUsec());
		u_.WriteTrace(buf);
		for(std::list<std::string>::iterator it=cur_func.other_paras.begin();it!=cur_func.other_paras.end();it++) {
		   std::string para = *it;
		   if(strcmp("exit-code",para.c_str()) == 0) {
			ArgumentHandle h; 
			int* exit_code = (int*)PopArgument(pt, &h, sizeof(uid_t));
			snprintf(buf,1024,"<exit-code>%d</exit-code>",*exit_code);
			u_.WriteTrace(buf);
		   }
		   if(strcmp("sockfd",para.c_str()) == 0) {
		  	ArgumentHandle h;
			int* sockfd = (int*)PopArgument(pt, &h, sizeof(int));
			snprintf(buf,1024, "<sockfd>%d</sockfd>",*sockfd);
			u_.WriteTrace(buf);
		   }	
		   if(strcmp("host-port",para.c_str()) == 0) {
			 ArgumentHandle h;
			 PopArgument(pt, &h, sizeof(int));
			 sockaddr** addr = (sockaddr**)PopArgument(pt, &h, sizeof(sockaddr*));
			 char host[256];
			 char service[256]; 
			  if (sp::GetAddress((sockaddr_storage*)*addr, host, 256, service, 256)) {
				snprintf(buf, 1024,"<host>%s</host><port>%s</port>",host,service);
				u_.WriteTrace(buf); 
			  }
		   }
		 if(strcmp("rem_host-port",para.c_str()) == 0) {
			 ArgumentHandle h;
			 int* fd = (int*)PopArgument(pt, &h, sizeof(int));
			 sockaddr_storage addr; 
			 if (sp::GetRemoteAddress(*fd, &addr)) { 
                 	     char host[256]; 
                             char service[256];                                                                  
                              if (sp::GetAddress((sockaddr_storage*)&addr, host, 256, service, 256)) {   
                                 snprintf(buf, 1024,"<host>%s</host><port>%s</port>",host,service); 
                                 u_.WriteTrace(buf);
		               }
  		       }
 		   }
		  if(strcmp("uid-name",para.c_str()) == 0) {
			ArgumentHandle h;
			uid_t* uid = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));  
			snprintf(buf, 1024,"<name>%s</name><id>%d</id>", u_.get_user_name(*uid).c_str(), *uid);
		        u_.WriteTrace(buf); 
			
		  }
		
           	  if(strcmp("gid-name",para.c_str()) == 0) {  		
			ArgumentHandle h;
			gid_t* gid = (gid_t*)PopArgument(pt, &h, sizeof(gid_t));
			 snprintf(buf, 1024,"<group-name>%s</group-name><gid>%d</gid>", u_.get_group_name(*gid).c_str(), *gid);
			 u_.WriteTrace(buf);    
		  }
  
		 if(strcmp("cmd",para.c_str()) == 0) {
			ArgumentHandle h;
			char** cmd = (char**)PopArgument(pt,&h,sizeof(void*));
			snprintf(buf,1024,"<cmd>%s</cmd>",*cmd);
			u_.WriteTrace(buf);			
		 }
		
		if(strcmp("socket-id",para.c_str()) == 0) {                                                             
                         ArgumentHandle h;
                         int* socket = (int*)PopArgument(pt, &h, sizeof(int));                                   
                         snprintf(buf, 1024,"<socket-id>%d</socket-id>",*socket);              
                         u_.WriteTrace(buf);      
 		  }
		 if(strcmp("file",para.c_str()) == 0 ) {
                         ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        snprintf(buf,1024,"<file>%s</file>",*filename);
                        u_.WriteTrace(buf);

                }
		if(strcmp("file",para.c_str()) == 0 ) {
                         ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        snprintf(buf,1024,"<file>%s</file>",*filename);
                        u_.WriteTrace(buf);

                }
                if(strcmp("file-owner-group",para.c_str()) == 0) {
                        struct stat info;
                        ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        stat(*filename,&info);
                        snprintf(buf,1024,"<owner> %d </owner>", info.st_uid);
                        u_.WriteTrace(buf);
                        snprintf(buf,1024,"<group> %d </group>", info.st_gid);
                        u_.WriteTrace(buf);
                }
                if(strcmp("file-mode",para.c_str()) == 0) {
                        struct stat info;
                        ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        stat(*filename,&info);
                        snprintf(buf,1024,"<mode> %lo</mode>",(unsigned long) info.st_mode);
                        u_.WriteTrace(buf);
                 }
		if(strcmp("file-descriptor-filename",para.c_str()) == 0) {
			 ArgumentHandle h;
                         int* fd = (int*)PopArgument(pt, &h, sizeof(int));
			 char filename[200];
                         char proclnk[200];
			 char filetype[200];
                         int MAXSIZE = 0xFFF, r;
                         sprintf(proclnk,"/proc/self/fd/%d",*fd);
                         r = readlink(proclnk,filename,MAXSIZE);
			 struct stat info;
			 stat(filename,&info);
			 if(S_ISREG(info.st_mode)) 
				strcpy(filetype, "REGULAR_FILE");
			 if(S_ISDIR(info.st_mode))
				strcpy(filetype,"DIRECTORY");
			 if(S_ISFIFO(info.st_mode))
				strcpy(filetype,"PIPE");
			 if(S_ISSOCK(info.st_mode))
				strcpy(filetype,"SOCKET");
                         if(r >= 0) {
                                filename[r]='\0';
                            //    snprintf(buf,1024,"<file-descriptor>%d</file-descriptor><file type=%s name= %s> </file>",*fd,filetype, filename);
 	                        snprintf(buf,1024,"<file-descriptor>%d</file-descriptor><file name= \"%s\"> </file>",*fd, filename);

                                u_.WriteTrace(buf);
                         }
		}


	} 

                       u_.WriteTrace("</trace>");
              
                  } 

 		
}

  for (Checkers::iterator i = checkers_.begin();
       i != checkers_.end(); i++) {
    (*i)->check(pt, f);
  }
}

void Mist::post_run(SpPoint* pt, SpFunction* f) {

   for(unsigned int i=0; i< post_inst_func.size();i++)
  {
        struct Inst_func cur_func = post_inst_func.at(i);
        if(f->name().compare(cur_func.func_name.c_str()) == 0 )
        {
                char buf[1024];
                snprintf(buf, 1024,"<trace type=\"%s\" time=\"%lu\">",cur_func.desc.c_str(), u_.GetUsec());
                u_.WriteTrace(buf);
                for(std::list<std::string>::iterator it=cur_func.other_paras.begin();it!=cur_func.other_paras.end();it++) {
                   std::string para = *it;
                   if(strcmp("socket-id",para.c_str()) == 0) {
                        ArgumentHandle h;
                        int socket = ReturnValue(pt);
                        snprintf(buf,1024,"<sockfd>%d</sockfd>",socket);
                        u_.WriteTrace(buf);
                   }
		
		if(strcmp("file",para.c_str()) == 0 ) {
			 ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        snprintf(buf,1024,"<file>\"%s\"</file>",*filename);
                        u_.WriteTrace(buf);

		}
	        
        	if(strcmp("file-owner-group",para.c_str()) == 0) {
                        struct stat info;
                        ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        stat(*filename,&info);
                        snprintf(buf,1024,"<owner> %d </owner>", info.st_uid);
                        u_.WriteTrace(buf);
                        snprintf(buf,1024,"<group> %d </group>", info.st_gid);
                        u_.WriteTrace(buf);
                                                
                }
		if(strcmp("file-mode",para.c_str()) == 0) {
                        struct stat info;
                        ArgumentHandle h;
                        char** filename = (char**)PopArgument(pt,&h,sizeof(void*));
                        stat(*filename,&info);
                        snprintf(buf,1024,"<mode> %lo</mode>",(unsigned long) info.st_mode);
                        u_.WriteTrace(buf);
                 }
		if(strcmp("return-fd",para.c_str()) == 0 ) {
			ArgumentHandle h;
			int fd = ReturnValue(pt);
			snprintf(buf,1024,"<file-descriptor>%d</file-descriptor>",fd);
                        u_.WriteTrace(buf);
		}
		if(strcmp("return-pid",para.c_str()) == 0) {
			ArgumentHandle h;
			int pid = ReturnValue(pt);
			if(pid >0) {
			snprintf(buf,1024,"<child-process-id>%d</child-process-id>",pid);
                        u_.WriteTrace(buf);
			}
			else {
			snprintf(buf,1024,"%s failed",cur_func.desc.c_str());
                        u_.WriteTrace(buf);
			}
		}
	/*	if(strcmp("",para.c_str()) ==0 ) {
			
    			ArgumentHandle h;
			char** path = (char**)PopArgument(pt, &h, sizeof(char*));
		        char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
			char*** envs = (char***)PopArgument(pt, &h, sizeof(char**));
    			FILE* fp = fopen(u_.TraceFileName().c_str(), "r+");
    
			    if (fp) {
			    fseek(fp, -19, SEEK_END);
			    fprintf(fp, "%s</trace></traces></process>", buf);
			    fclose(fp);
			    } else {
			      char trace_file_name[255];
			      unsigned long seq = MistUtils::GetUsec();
			      snprintf(trace_file_name, 255, "/tmp/%d-%.14lu.xml", getpid(), seq);
			      fp = fopen(trace_file_name, "w");
			      if (fp) {
			        fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><process>"
		                "<head></head><traces>%s</trace></traces></process>", buf);
			        fclose(fp);
      				}
    			   }
			    // Execve!
			    u_.ChangeTraceFile();
	
		}*/
	     }
                       u_.WriteTrace("</trace>");
	}

  }
 
  for (Checkers::iterator i = checkers_.begin();
       i != checkers_.end(); i++) {
    (*i)->post_check(pt, f);
  }
}

}
