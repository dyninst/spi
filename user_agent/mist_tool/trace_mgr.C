#include <sys/stat.h>

#include "SpInc.h"
#include "trace_mgr.h"

namespace mist {

//////////////////////////////////////////////////////////////////////

TraceMgr::TraceMgr() {
  OpenFile();
  
}

void
TraceMgr::ChangeTraceFile() {
  OpenFile();
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
  fflush(fp_);
  fsync(fileno(fp_));
  fclose(fp_);
}

////////////////////////////////////////////////////////////////////// 
std::string
TraceMgr::TraceFileName() const {
  return filename_;
}

//////////////////////////////////////////////////////////////////////
// Naming convention:
// 1. trace file: /tmp/pid-seq.xml (seq starts from 1)
// 2. sequence number file: /tmp/pid-seq-num
//
// Basic steps:
// 1. Check if /tmp/pid-seq-num exists, if not create it
// 2. Read pid-seq-num to get the sequence number
// 3. Construct the trace file name based on the sequence number
// 4. Open trace file for write
// 5. Increment the sequence number
// 6. Write sequence number into /tmp/pid-seq-num
// 7. Write xml template into trace file

void
TraceMgr::OpenFile() {
  // Get seq
  char seq_file_name[255];
  snprintf(seq_file_name, 255, "/tmp/%d-seq-num", getpid());

  FILE* fp_seq = NULL;
  char linebuf[255];

  struct stat s;
  if (stat(seq_file_name, &s) == 0) {
    sp_debug("%s exists\n", seq_file_name);

    fp_seq = fopen(seq_file_name, "rw");
    if (fp_seq == NULL) {
      sp_perror("fail to open %s\n", seq_file_name);
    }
    if (fgets(linebuf, 255, fp_seq) == NULL) {
      sp_perror("fail to read %s\n", seq_file_name);
    }
  } else {
    sp_debug("%s not exists\n", seq_file_name);

    fp_seq = fopen(seq_file_name, "w");
    if (fp_seq == NULL) {
      sp_perror("fail to open %s\n", seq_file_name);
    }
    fprintf(fp_seq, "1");
    fflush(fp_seq);
    strcpy(linebuf, "1");
  }
  int seq = atoi(linebuf);

  // Construct trace file name
  char trace_file_name[255];
  snprintf(trace_file_name, 255, "/tmp/%d-%d.xml", getpid(), seq);
  filename_ = trace_file_name;
  fp_ = fopen(trace_file_name, "w");
  if (fp_ == NULL) {
    sp_perror("fail to open %s\n", trace_file_name);
  }
  setbuf(fp_, NULL);

  // Write back seq
  ++seq;
  rewind(fp_seq);
  fprintf(fp_seq, "%d", seq);
  fclose(fp_seq);

  // Write initial data
  string init = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
  init += "<process><head>";
  init += "</head></process>";
  WriteString(init);
}

}

