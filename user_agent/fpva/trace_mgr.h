#ifndef _TRACE_MGR_H_
#define _TRACE_MGR_H_

#include <libxml/parser.h>
#include <libxml/tree.h>  // adds dependency: libxml2
#include <stdio.h>

#include <fstream>
#include <string>

namespace fpva {

/**
 * A struct to store information of a call trace
 * The checkers pass this into the trace manager to write it into xml file
 */
struct CallTrace {
  std::string functionName;
  std::string timeStamp;
  std::map<std::string, std::string> parameters;
};

// Trace Manager
class TraceMgr {
 public:
  TraceMgr();
  ~TraceMgr();

  void NewDoc();
  bool AddTrace(CallTrace* trace);
  void Fabricate();
  std::string TraceFileName() const;
  void PrintCurrentProc();
  void PrintParentProc();
  void PrintUserInfo();
  void Flush();

 private:
  std::string filename_;
  std::string working_dir_;
  xmlDocPtr xmlDoc_;
  xmlNodePtr head_node_;
  xmlNodePtr traces_node_;
  bool dirty_;
};

}  // namespace fpva
#endif /* _TRACE_MGR_H_ */
