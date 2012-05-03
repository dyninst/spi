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

  private:
  std::string filename_;
  FILE* fp_;

  void Complete();
  void WriteString(std::string str);
};

}
#endif /* _TRACE_MGR_H_ */
