#ifndef _TRACE_MGR_H_
#define _TRACE_MGR_H_

#include <stdio.h>
#include <string>

namespace mist {
  
// Trace Manager
class TraceMgr {
  public:
  TraceMgr();
  ~TraceMgr();

  void WriteHeader(std::string header);
  void WriteTrace(std::string trace);
  void WriteString(long pos,
                   std::string str);
  void WriteString(std::string str);
  void CloseTrace();
  std::string TraceFileName() const;
  
  void ChangeTraceFile();
  void XMLEncode(std::string& str);
  private:
  void OpenFile();
  std::string filename_;
  FILE* fp_;

};

}
#endif /* _TRACE_MGR_H_ */
