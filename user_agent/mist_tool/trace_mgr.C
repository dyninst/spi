#include "SpInc.h"
#include "trace_mgr.h"

namespace mist {

//////////////////////////////////////////////////////////////////////

TraceMgr::TraceMgr() {
  if (getenv("MIST_TRACE_FILE")) {
    filename_ = getenv("MIST_TRACE_FILE");
  } else {
    char buf[1024];
    snprintf(buf, 1024, "%d-trace.xml", getpid());
    filename_ = buf;
  }
  
  fp_ = fopen(filename_.c_str(), "w");
  if (fp_ == NULL) {
    sp_perror("Failed to write %s", filename_.c_str());
  }

  string init = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
  init += "<process><head>";
  init += "</head></process>";
  WriteString(init);
}

//////////////////////////////////////////////////////////////////////

TraceMgr::~TraceMgr() {
  fclose(fp_);
}

//////////////////////////////////////////////////////////////////////

// Assumption for the current layout of xml file:
// <?xml ... ?><process><head></head></process>
void
TraceMgr::WriteHeader(std::string header) {
  WriteString(-17, header);
  WriteString("</head></process>");
}

//////////////////////////////////////////////////////////////////////

// Assumption for the current layout of xml file:
// <?xml ... ?><process><head></head><traces><trace></trace></traces></process>
void
TraceMgr::WriteTrace(std::string trace) {
  trace += "</traces></process>";
  WriteString(-19, trace);
}

//////////////////////////////////////////////////////////////////////

void
TraceMgr::WriteString(long pos,
                      std::string str) {
  fseek(fp_, pos, SEEK_END);
  fprintf(fp_, "%s", str.c_str());
  fflush(fp_);
}

//////////////////////////////////////////////////////////////////////

void
TraceMgr::WriteString(std::string str) {
  fprintf(fp_, "%s", str.c_str());
  fflush(fp_);
}


}

