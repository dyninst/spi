/*
  1. Attach
  2. Inject
  3. Activate
  4. Force to exe
 */

/*
  Plan:
    1. mutatee.exe: executes in a while loop, sleep for every 3 seconds
    2. agent.so: agent, written in c first
       2.1 hello()
    3. injector.exe: attach agent.so to mutatee.exe, and execute hello()
 */

#include "Process.h"
#include "Event.h"
#include <elf.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <elf.h>
#include <unistd.h>
#include <errno.h>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

#define MEMORY_ONLY  "[memory]"

static char sysexit_code[] = {
  0xb8, 0x01, 0x0, 0x0, 0x0,  /* mov $121, %eax */
  0xbb, 0x79, 0x0, 0x0, 0x0,  /* call exit */
  0xcd, 0x80                  /* int $80   */
};


enum {
  OFF_ARGS = 2,
  OFF_DLOPEN = 7,
  OFF_DLRET = 11
};
static char dlopen_code[] = {

  0x50,                       /* pushl %eax */

  0x68, 0x0, 0x0, 0x0, 0x0,   /* pushl args */
  0xe8, 0x0, 0x0, 0x0, 0x0,   /* call do_dlopen */
  0x58,                       /* popl %eax */

  0x58,                       /* popl %eax */
  0xc3
};

typedef struct dlopen_args {
  const char *libname;
  int mode;
  //void* result;
  //void* caller;
  void* link_map;
} dlopen_args_t;

typedef void *(*DYNINST_do_dlopen)(dlopen_args_t *);

class Injector {
  public:
    Injector(Dyninst::PID pid) : pid_(pid) {
      proc_ = Process::attachProcess(pid);
    }

    void inject(char* code, size_t size) {
      if (!proc_->stopProc()) return;
      runCode(code, size);
    }

    void inject(char* code, char* lib, size_t size) {
      if (!proc_->stopProc()) return;

      // save eip to stack
      ThreadPool& tp = proc_->threads();
      Thread::ptr t = tp.getInitialThread();
      MachRegisterVal eip;
      MachRegisterVal esp;
      t->getRegister(x86::eip, eip);
      t->getRegister(x86::esp, esp);
      esp -= 4;
      proc_->writeMemory(esp, &eip, 4);
      t->setRegister(x86::esp, esp);

      // Find do_dlopen's address
      Dyninst::Address do_dlopen_addr = find_do_dlopen();
      cerr << "do_dlopen is at: " << std::hex << do_dlopen_addr << "\n";

      // Store library name in mutatee's heap
      Dyninst::Address lib_addr = proc_->mallocMemory(strlen(lib) + 1);
      proc_->writeMemory(lib_addr, lib, strlen(lib) + 1);
      fprintf(stderr, "lib_name is at: %x\n", lib_addr);

      // Store do_dlopen's argument in mutatee's heap
      dlopen_args_t args;
      Dyninst::Address args_addr = proc_->mallocMemory(sizeof(args));
      Dyninst::Address code_addr = proc_->mallocMemory(size+1);
      args.libname = (char*)lib_addr;
      args.mode = RTLD_NOW | RTLD_GLOBAL;
      args.link_map = 0;
      //args.result = 0;
      //args.caller = (void*)do_dlopen_addr;
      //args.caller = (void*)(code_addr+OFF_DLRET);
      //args.caller = (void*)(do_dlopen_addr - (code_addr+OFF_DLRET));
      proc_->writeMemory(args_addr, &args, sizeof(args));
      fprintf(stderr, "args is at: %x\n", args_addr);

      // Setup code and inject code into mutatee's heap
      unsigned long* p = (unsigned long*)&code[OFF_DLOPEN];
      *p = (unsigned long)do_dlopen_addr - (unsigned long)(code_addr + OFF_DLRET);
      fprintf(stderr, "*p: %x\n", *p);
      p = (unsigned long*)&code[OFF_ARGS];
      *p = (unsigned long)args_addr;
      proc_->writeMemory((Dyninst::Address)code_addr, code, size);
      fprintf(stderr, "code_addr is at: %x\n", code_addr);

      // Here we go!
      IRPC::ptr irpc = IRPC::createIRPC(code, size, code_addr, true);
      if (proc_->postIRPC(irpc)) {
        cerr << "Injector: post irpc successfully!\n";
      }
      proc_->continueProc();
      proc_->detach();
      Process::handleEvents(true);
    }

  protected:
    Process::ptr proc_;
    Dyninst::PID pid_;

    void runCode(char* code, size_t size) {
      Dyninst::Address addr = proc_->mallocMemory(size+1);
      IRPC::ptr irpc = IRPC::createIRPC(code, size, addr, true);
      if (proc_->postIRPC(irpc)) {
        cerr << "Injector: post irpc successfully!\n";
      }
      proc_->continueProc();
      cerr << std::hex << irpc->getAddress() << "\n";
      Process::handleEvents(true);
    }

    struct mm {
      char name[256];
      unsigned long start, end;
    };
    struct symlist {
      Elf32_Sym *sym;       /* symbols */
      char *str;            /* symbol strings */
      unsigned num;         /* number of symbols */
    };
    struct symtab {
      struct symlist *st;    /* "static" symbols */
      struct symlist *dyn;   /* dynamic symbols */
    };
    typedef struct symtab *symtab_t;

    int load_memmap(struct mm *mm, int *nmmp) {
      char raw[10000];
      char name[256];
      char *p;
      unsigned long start, end;
      struct mm *m;
      int nmm = 0;
      int fd, rv;
      int i;

      sprintf(raw, "/proc/%d/maps", pid_);
      fd = open(raw, O_RDONLY);
      if (0 > fd) {
        fprintf(stderr, "Can't open %s for reading\n", raw);
        return -1;
      }

      /* Zero to ensure data is null terminated */
      memset(raw, 0, sizeof(raw));

      p = raw;
      while (1) {
        rv = read(fd, p, sizeof(raw)-(p-raw));
        if (0 > rv) {
          perror("read");
          return -1;
        }
        if (0 == rv)
          break;
        p += rv;
        if (p-raw >= sizeof(raw)) {
          fprintf(stderr, "Too many memory mapping\n");
          return -1;
        }
     }
     close(fd);

     p = strtok(raw, "\n");
     m = mm;
     while (p) {
       /* parse current map line */
       rv = sscanf(p, "%08lx-%08lx %*s %*s %*s %*s %s\n",
                   &start, &end, name);
       p = strtok(NULL, "\n");
       if (rv == 2) {
         m = &mm[nmm++];
         m->start = start;
         m->end = end;
         strcpy(m->name, MEMORY_ONLY);
         continue;
       }

       /* search backward for other mapping with same name */
       for (i = nmm-1; i >= 0; i--) {
         m = &mm[i];
         if (!strcmp(m->name, name))
           break;
       }

       if (i >= 0) {
         if (start < m->start)
           m->start = start;
         if (end > m->end)
           m->end = end;
       } else {
         /* new entry */
         m = &mm[nmm++];
         m->start = start;
         m->end = end;
         strcpy(m->name, name);
       }
     }
     *nmmp = nmm;
     return 0;
    }

  symlist *
  get_syms(int fd, Elf32_Shdr *symh, Elf32_Shdr *strh) {
    struct symlist *sl, *ret;
    int rv;

    ret = NULL;
    sl = (struct symlist *) xmalloc(sizeof(struct symlist));
    sl->str = NULL;
    sl->sym = NULL;

    /* sanity */
    if (symh->sh_size % sizeof(Elf32_Sym)) {
      fprintf(stderr, "elf_error\n");
      goto out;
    }

    /* symbol table */
    sl->num = symh->sh_size / sizeof(Elf32_Sym);
    sl->sym = (Elf32_Sym *) xmalloc(symh->sh_size);
    rv = pread(fd, sl->sym, symh->sh_size, symh->sh_offset);
    if (0 > rv) {
      perror("read");
      goto out;
    }
    if (rv != symh->sh_size) {
      fprintf(stderr, "elf error\n");
      goto out;
    }

    /* string table */
    sl->str = (char *) xmalloc(strh->sh_size);
    rv = pread(fd, sl->str, strh->sh_size, strh->sh_offset);
    if (0 > rv) {
      perror("read");
      goto out;
    }
    if (rv != strh->sh_size) {
      fprintf(stderr, "elf error");
      goto out;
    }

    ret = sl;
  out:
    return ret;
  }

  int do_load(int fd, symtab_t sym) {
    int rv;
    size_t size;
    Elf32_Ehdr ehdr;
    Elf32_Shdr *shdr = NULL, *p;
    Elf32_Shdr *dynsymh, *dynstrh;
    Elf32_Shdr *symh, *strh;
    char *shstrtab = NULL;
    int i;
    int ret = -1;

    /* elf header */
    rv = read(fd, &ehdr, sizeof(ehdr));
    if (0 > rv) {
      perror("read");
      goto out;
    }
    if (rv != sizeof(ehdr)) {
      fprintf(stderr, "elf error\n");
      goto out;
    }
    if (strncmp((const char*)(ELFMAG), (const char*)(ehdr.e_ident), SELFMAG)) { /* sanity */
      fprintf(stderr, "not an elf\n");
      goto out;
    }
    if (sizeof(Elf32_Shdr) != ehdr.e_shentsize) { /* sanity */
      fprintf(stderr, "elf error\n");
      goto out;
    }

    /* section header table */
    size = ehdr.e_shentsize * ehdr.e_shnum;
    shdr = (Elf32_Shdr *) xmalloc(size);
    rv = pread(fd, shdr, size, ehdr.e_shoff);
    if (0 > rv) {
      perror("read");
      goto out;
    }
    if (rv != size) {
      fprintf(stderr, "elf error");
      goto out;
    }

    /* section header string table */
    size = shdr[ehdr.e_shstrndx].sh_size;
    shstrtab = (char *) xmalloc(size);
    rv = pread(fd, shstrtab, size, shdr[ehdr.e_shstrndx].sh_offset);
    if (0 > rv) {
      perror("read");
      goto out;
    }
    if (rv != size) {
      fprintf(stderr, "elf error\n");
      goto out;
    }

    /* symbol table headers */
    symh = dynsymh = NULL;
    strh = dynstrh = NULL;
    for (i = 0, p = shdr; i < ehdr.e_shnum; i++, p++)
      if (SHT_SYMTAB == p->sh_type) {
        if (symh) {
          fprintf(stderr, "too many symbol tables\n");
          goto out;
        }
        symh = p;
      } else if (SHT_DYNSYM == p->sh_type) {
        if (dynsymh) {
          fprintf(stderr, "too many symbol tables\n");
          goto out;
        }
        dynsymh = p;
      } else if (SHT_STRTAB == p->sh_type
                 && !strncmp(shstrtab+p->sh_name, ".strtab", 7)) {
        if (strh) {
          fprintf(stderr, "too many string tables\n");
          goto out;
        }
        strh = p;
      } else if (SHT_STRTAB == p->sh_type
                 && !strncmp(shstrtab+p->sh_name, ".dynstr", 7)) {
        if (dynstrh) {
          fprintf(stderr, "too many string tables\n");
          goto out;
        }
        dynstrh = p;
      }
    /* sanity checks */
    if ((!dynsymh && dynstrh) || (dynsymh && !dynstrh)) {
      fprintf(stderr, "bad dynamic symbol table");
      goto out;
    }
    if ((!symh && strh) || (symh && !strh)) {
      fprintf(stderr, "bad symbol table");
      goto out;
    }
    if (!dynsymh && !symh) {
      fprintf(stderr, "no symbol table");
      goto out;
    }

    /* symbol tables */
    if (dynsymh)
      sym->dyn = get_syms(fd, dynsymh, dynstrh);
    if (symh)
      sym->st = get_syms(fd, symh, strh);
    ret = 0;

  out:
    free(shstrtab);
    free(shdr);
    return ret;
  }

    void *xmalloc(size_t size) {
      void *p;
      p = malloc(size);
      if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
      }
      return p;
    }

    symtab_t load_symtab(char *filename) {
      int fd;
      symtab_t sym;
      sym = (symtab_t) xmalloc(sizeof(symtab));

      memset(sym, 0, sizeof(symtab));
      fd = open(filename, O_RDONLY);
      if (0 > fd) {
        perror("open");
        return NULL;
      }

      if (0 > do_load(fd, sym)) {
        fprintf(stderr, "Error ELF parsing %s\n", filename);
        free(sym);
        sym = NULL;
      }
      close(fd);
      return sym;
    }


    int find_ld(char *name, int len, unsigned long *start,
                  struct mm *mm, int nmm) {
      int i;
      struct mm *m;
      char *p;
      for (i = 0, m = mm; i < nmm; i++, m++) {
        if (!strcmp(m->name, MEMORY_ONLY))
          continue;
        p = strrchr(m->name, '/');
        if (!p)
          continue;
        p++;
        if (strncmp("ld", p, 2))
          continue;
        p += 2;

        /* here comes our crude test -> 'libc.so' or 'libc-[0-9]' */
        if (!strncmp(".so", p, 3) || (p[0] == '-' && isdigit(p[1])))
          break;
      }
      if (i >= nmm)
      /* not found */
        return -1;

      *start = m->start;
      strncpy(name, m->name, len);
      if (strlen(m->name) >= len)
        name[len-1] = '\0';
      return 0;
    }

    int find_libc(char *name, int len, unsigned long *start,
                  struct mm *mm, int nmm) {
      int i;
      struct mm *m;
      char *p;
      for (i = 0, m = mm; i < nmm; i++, m++) {
        if (!strcmp(m->name, MEMORY_ONLY))
          continue;
        p = strrchr(m->name, '/');
        if (!p)
          continue;
        p++;
        if (strncmp("libc", p, 4))
          continue;
        p += 4;

        /* here comes our crude test -> 'libc.so' or 'libc-[0-9]' */
        if (!strncmp(".so", p, 3) || (p[0] == '-' && isdigit(p[1])))
          break;
      }
      if (i >= nmm)
      /* not found */
        return -1;

      *start = m->start;
      strncpy(name, m->name, len);
      if (strlen(m->name) >= len)
        name[len-1] = '\0';
      return 0;
    }

    Dyninst::Address find_dlopen() {
      Dyninst::Address addr = 0;
      struct mm mm[50];
      unsigned long ldaddr;
      int nmm;
      char ld[256];
      symtab_t s;

      if (0 > load_memmap(mm, &nmm)) {
        fprintf(stderr, "cannot read memory map\n");
        return -1;
      }

      if (0 > find_ld(ld, sizeof(ld), &ldaddr, mm, nmm)) {
        fprintf(stderr, "cannot find ld.so\n");
        return -1;
      }
      s = load_symtab(ld);
      if (!s) {
        fprintf(stderr, "cannot read symbol table\n");
        return -1;
      }

      if (0 > lookup_func_sym(s, "_dl_open", &addr)) {
        fprintf(stderr, "cannot find _dl_open\n");
      return -1;
      }
      cerr << "relative addr: " << std::hex << addr << "\n";
      addr += ldaddr;
      return addr;
    }

    Dyninst::Address find_do_dlopen() {
      Dyninst::Address addr = 0;
      struct mm mm[50];
      unsigned long ldaddr;
      int nmm;
      char ld[256];
      symtab_t s;

      if (0 > load_memmap(mm, &nmm)) {
        fprintf(stderr, "cannot read memory map\n");
        return -1;
      }

      if (0 > find_libc(ld, sizeof(ld), &ldaddr, mm, nmm)) {
        fprintf(stderr, "cannot find ld.so\n");
        return -1;
      }
      s = load_symtab(ld);
      if (!s) {
        fprintf(stderr, "cannot read symbol table\n");
        return -1;
      }

      if (0 > lookup_func_sym(s, "do_dlopen", &addr)) {
        fprintf(stderr, "cannot find do_dlopen\n");
      return -1;
      }
      cerr << "relative addr: " << std::hex << addr << "\n";
      addr += ldaddr;
      return addr;
    }

 int
lookup2(struct symlist *sl, unsigned char type,
        char *name, unsigned long *val)
{
  Elf32_Sym *p;
  int len;
  int i;

  len = strlen(name);
  for (i = 0, p = sl->sym; i < sl->num; i++, p++)
    if (!strncmp(sl->str+p->st_name, name, len)
        && ELF32_ST_TYPE(p->st_info) == type) {
      *val = p->st_value;
      return 0;
    }
  return -1;
}

int
lookup_sym(symtab_t s, unsigned char type,
           char *name, unsigned long *val)
{
  if (s->dyn && !lookup2(s->dyn, type, name, val))
    return 0;
  if (s->st && !lookup2(s->st, type, name, val))
    return 0;
  return -1;
}

int
lookup_func_sym(symtab_t s, char *name, unsigned long *val)
{
  return lookup_sym(s, STT_FUNC, name, val);
}

};




int main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: Injector pid lib\n";
    return 0;
  }

  Dyninst::PID pid = atoi(argv[1]);
  Injector injector(pid);
  // injector.inject(sysexit_code, sizeof(sysexit_code));
  injector.inject(dlopen_code, argv[2], sizeof(dlopen_code));
  //injector.self();

  return 0;
}
