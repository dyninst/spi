#include "SpInc.h"
#include "trace_mgr.h"

namespace mist {

//////////////////////////////////////////////////////////////////////

TraceMgr::TraceMgr() {
  if (getenv("MIST_TRACE_DIR")) {
    char buf[1024];
    snprintf(buf, 1024, "%s/%d-trace.xml",
             getenv("MIST_TRACE_DIR"), getpid());
    filename_ = buf;
  } else {
    char buf[1024];
    srand(time(0));
    snprintf(buf, 1024, "/tmp/%d-trace%d.xml", getpid(), rand());
    filename_ = buf;
  }
  
  fp_ = fopen(filename_.c_str(), "w");
  if (fp_ == NULL) {
    sp_perror("Failed to write %s", filename_.c_str());
  }
  setbuf(fp_, NULL);
  
  string init = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
  init += "<process><head>";
  init += "</head></process>";
  WriteString(init);
}

void
TraceMgr::ChangeTraceFile() {
  char buf[1024];
  srand(time(0));
  snprintf(buf, 1024, "/tmp/%d-trace%d.xml", getpid(), rand());
  filename_ = buf;
  
  fp_ = fopen(filename_.c_str(), "w");
  if (fp_ == NULL) {
    sp_perror("Failed to write %s", filename_.c_str());
  }
  setbuf(fp_, NULL);

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
  fflush(fp_);
  fsync(fileno(fp_));
}

//////////////////////////////////////////////////////////////////////

void
TraceMgr::WriteString(long pos,
                      std::string str) {
  fseek(fp_, pos, SEEK_END);
  fprintf(fp_, "%s", str.c_str());
  fflush(fp_);
  fsync(fileno(fp_));
}

//////////////////////////////////////////////////////////////////////

void
TraceMgr::WriteString(std::string str) {
  fprintf(fp_, "%s", str.c_str());
  fflush(fp_);
  fsync(fileno(fp_));
}

////////////////////////////////////////////////////////////////////// 
void
TraceMgr::XMLEncode(std::string& data) {
  std::string buffer;
  buffer.reserve(data.size());
  for(size_t pos = 0; pos != data.size(); ++pos) {
    switch(data[pos]) {
        case '&':  buffer.append("&amp;");       break;
        case '\"': buffer.append("&quot;");      break;
        case '\'': buffer.append("&apos;");      break;
        case '<':  buffer.append("&lt;");        break;
        case '>':  buffer.append("&gt;");        break;
        default:   buffer.append(1, data[pos]); break;
      }
  }
  data.swap(buffer);
}

void
TraceMgr::CloseTrace() {
  fsync(fileno(fp_));
  fclose(fp_);
}

}

