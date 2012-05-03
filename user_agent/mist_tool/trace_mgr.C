#include "SpInc.h"
#include "trace_mgr.h"

namespace mist {

TraceMgr::TraceMgr() {
  if (getenv("MIST_TRACE_FILE")) {
    filename_ = getenv("MIST_TRACE_FILE");
  } else {
    filename_ = "trace.xml";
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

TraceMgr::~TraceMgr() {
  fclose(fp_);
}

// Assumption for the current layout of xml file:
// <?xml ... ?><process><head></head></process>
void TraceMgr::WriteHeader(std::string header) {
  fseek(fp_, -17, SEEK_END);
  WriteString(header);
  WriteString("</head></process>");
}

void TraceMgr::Complete() {
  WriteString("</traces></process>");
}

void TraceMgr::WriteString(std::string str) {
  fprintf(fp_, "%s", str.c_str());
  fflush(fp_);
}

}
