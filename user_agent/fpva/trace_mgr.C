#include <errno.h>
#include <sys/stat.h>

#include "SpInc.h"
#include "fpva_utils.h"
#include "trace_mgr.h"

namespace fpva {

//////////////////////////////////////////////////////////////////////

TraceMgr::TraceMgr() {
  sp_print("TraceMgr init for pid=%d\n", getpid());
  // first figure out where to store trace files
  if (getenv("SP_DIR") && getenv("PLATFORM")) {
    working_dir_ = getenv("SP_DIR");
    working_dir_ += "/";
    working_dir_ += getenv("PLATFORM");
  } else {
    sp_perror(
        "SP_DIR and PLATFORM environment variable is not set, using root as "
        "the working dir");
    working_dir_ = "/";
  }
  this->NewDoc();
}

//////////////////////////////////////////////////////////////////////

TraceMgr::~TraceMgr() { Flush(); }

//////////////////////////////////////////////////////////////////////

/**
 * Append a trace under the "traces" node in the xml file
 * First create a new "trace" node, and add two attributes, "type" and "time"
 * Then add paramters as the text children
 */
bool TraceMgr::AddTrace(CallTrace* trace) {
  if (xmlNodePtr newTrace =
          xmlNewChild(this->traces_node_, NULL, BAD_CAST "trace", NULL)) {
    xmlNewProp(newTrace, BAD_CAST "type", BAD_CAST trace->functionName.c_str());
    xmlNewProp(newTrace, BAD_CAST "time", BAD_CAST trace->timeStamp.c_str());
    for (std::map<std::string, std::string>::iterator i =
             trace->parameters.begin();
         i != trace->parameters.end(); i++) {
      xmlNewTextChild(newTrace, NULL, BAD_CAST i->first.c_str(),
                      BAD_CAST i->second.c_str());
    }
    dirty_ = true;
    return true;
  }
  return false;
}

void TraceMgr::Fabricate() {
  if (xmlSaveFormatFileEnc(this->filename_.c_str(), this->xmlDoc_, "UTF-8", 1) <
      0) {
    sp_perror("Failed to save xml to disk");
  }
}

void TraceMgr::Flush() {
  if (this->dirty_) {
    this->Fabricate();
    this->dirty_ = false;
  }
}

//////////////////////////////////////////////////////////////////////
std::string TraceMgr::TraceFileName() const { return filename_; }

//////////////////////////////////////////////////////////////////////
// Naming convention:
//
// trace file: /tmp/pid-seq.xml (seq has 14 digits, which is from
//               MistUtils::GetUsec())
//

void TraceMgr::NewDoc() {
  sp_print("NewDoc for pid=%d:%s\n", getpid(), sp::GetExeName().c_str());
  // Get seq
  unsigned long seq = FpvaUtils::GetUsec();

  // Construct trace file name
  char trace_file_name[256];
  snprintf(trace_file_name, 256, "%s/tmp/spi/%d-%.14lu.xml",
           working_dir_.c_str(), getpid(), seq);
  this->filename_ = trace_file_name;
  this->xmlDoc_ = xmlNewDoc((const xmlChar*)"1.0");
  if (xmlDoc_ == NULL) {
    sp_perror("fail to create new xml doc");
    return;
  }

  // Write initial data, create root node "program"
  // create "head" and "trace" under root
  xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "program");
  xmlDocSetRootElement(xmlDoc_, root);
  this->head_node_ = xmlNewChild(root, NULL, BAD_CAST "head", NULL);
  this->traces_node_ = xmlNewChild(root, NULL, BAD_CAST "traces", NULL);
  this->dirty_ = true;
}

/**
 * This function adds "pid", "exe_name", "working_dir", "host" nodes
 * under the "head" node
 */
void TraceMgr::PrintCurrentProc() {
  xmlNewChild(this->head_node_, NULL, BAD_CAST "pid",
              BAD_CAST std::to_string(getpid()).c_str());

  // Get exename
  char path[256];
  snprintf(path, 256, "/proc/%d/cmdline", getpid());
  std::ifstream cmdf(path, std::ifstream::in);
  string exename;
  if (std::getline(cmdf, exename)) {
    xmlNewChild(this->head_node_, NULL, BAD_CAST "exe_name",
                BAD_CAST exename.c_str());
  } else {
    //sp_perror("Failed to get exename from procfs cmdline");
    snprintf(path, 256, "/proc/%d/stat", getpid());
    std::ifstream cmdf2(path, std::ifstream::in);
    string pid;
    cmdf2 >> pid;
    cmdf2 >> exename;
    exename = exename.substr(1, exename.size()-2);
    xmlNewChild(this->head_node_, NULL, BAD_CAST "exe_name",
		BAD_CAST exename.c_str());
  }
  cmdf.close();

  // get working directory
  char buf[256];
  if (getcwd(buf, 256) != NULL) {
    xmlNewChild(this->head_node_, NULL, BAD_CAST "working_dir", BAD_CAST buf);
  } else {
    sp_perror("Failed to getcwd");
  }

  // Host ip
  sp::GetIPv4Addr(buf, 256);
  xmlNewChild(this->head_node_, NULL, BAD_CAST "host", BAD_CAST buf);
  this->dirty_ = true;
}

/**
 * This function writes parent pid and exe_name node to the head
 */
void TraceMgr::PrintParentProc() {
  xmlNodePtr parent = xmlNewChild(this->head_node_, NULL, BAD_CAST "parent", NULL);
  xmlNewChild(parent, NULL, BAD_CAST "parent_pid",
              BAD_CAST std::to_string(getppid()).c_str());
  char path[256];
  snprintf(path, 256, "/proc/%d/cmdline", getppid());
  std::ifstream cmdf(path, std::ifstream::in);
  string exename;
  if (std::getline(cmdf, exename)) {
    xmlNewChild(parent, NULL, BAD_CAST "parent_exe",
              BAD_CAST exename.c_str());
  }
  else {
    snprintf(path, 256, "/proc/%d/stat", getppid());
    std::ifstream cmdf2(path, std::ifstream::in);
    string pid;
    cmdf2 >> pid;
    cmdf2 >> exename;
    exename = exename.substr(1, exename.size()-2);
    xmlNewChild(parent, NULL, BAD_CAST "parent_exe",
             BAD_CAST exename.c_str());
  }
  this->dirty_ = true;
}

/**
 * This function writes uid, euid, gid, egid to the head node
 */
void TraceMgr::PrintUserInfo() {
  xmlNodePtr ruser =
      xmlNewChild(this->head_node_, NULL, BAD_CAST "real_user", NULL);
  xmlNewChild(ruser, NULL, BAD_CAST "name",
              BAD_CAST FpvaUtils::GetUserName(getuid()).c_str());
  xmlNewChild(ruser, NULL, BAD_CAST "id", BAD_CAST std::to_string(getuid()).c_str());

  xmlNodePtr euser =
      xmlNewChild(this->head_node_, NULL, BAD_CAST "effective_user", NULL);
  xmlNewChild(euser, NULL, BAD_CAST "name",
              BAD_CAST FpvaUtils::GetUserName(geteuid()).c_str());
  xmlNewChild(euser, NULL, BAD_CAST "id",
              BAD_CAST std::to_string(geteuid()).c_str());

  xmlNodePtr rgroup =
      xmlNewChild(this->head_node_, NULL, BAD_CAST "real_group", NULL);
  xmlNewChild(rgroup, NULL, BAD_CAST "name",
              BAD_CAST FpvaUtils::GetUserName(getgid()).c_str());
  xmlNewChild(rgroup, NULL, BAD_CAST "id",
              BAD_CAST std::to_string(getgid()).c_str());

  xmlNodePtr egroup =
      xmlNewChild(this->head_node_, NULL, BAD_CAST "effective_group", NULL);
  xmlNewChild(egroup, NULL, BAD_CAST "name",
              BAD_CAST FpvaUtils::GetUserName(getegid()).c_str());
  xmlNewChild(egroup, NULL, BAD_CAST "id",
              BAD_CAST std::to_string(getegid()).c_str());
  this->dirty_ = true;
}

}  // namespace fpva
