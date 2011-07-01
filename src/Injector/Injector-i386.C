#include "Injector.h"
#include "Common.h"

using sp::Injector;
using Dyninst::ProcControlAPI::ThreadPool;
using Dyninst::ProcControlAPI::Thread;
using Dyninst::MachRegisterVal;

/* Save %eip to stack. */
void Injector::save_pc() {
  ThreadPool& tp = proc_->threads();
  Thread::ptr t = tp.getInitialThread();
  MachRegisterVal eip;
  MachRegisterVal esp;
  t->getRegister(Dyninst::x86::eip, eip);
  t->getRegister(Dyninst::x86::esp, esp);
  esp -= 4;
  proc_->writeMemory(esp, &eip, 4);
  t->setRegister(Dyninst::x86::esp, esp);
}

struct mm {
  char name[MAX_LEN];
  ulong start;
  ulong end;
};
#define MEMORY_ONLY  "[memory]"

/* Load and parse /proc/$pid/maps into a list of module names and address ranges */
static int load_memmap(mm *mmap, int *nmmp, Dyninst::PID pid) {
  char raw[MAX_BUF_LEN];
  sprintf(raw, "/proc/%d/maps", pid);
  int fd = open(raw, O_RDONLY);
  if (0 > fd) {
    fprintf(stderr, "ERROR: Can't open %s for reading\n", raw);
    return -1;
  }

  /* Zero to ensure data is null terminated */
  memset(raw, 0, sizeof(raw));
  char* p = raw;
  int rv;
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
      fprintf(stderr, "ERROR: Too many memory mapping\n");
      return -1;
    }
  }
  close(fd);

  p = strtok(raw, "\n");
  mm *m = mmap;
  int nmm = 0;
  ulong start;
  ulong end;
  char name[MAX_LEN];
  int i;
  while (p) {
    /* parse current map line */
    rv = sscanf(p, "%08lx-%08lx %*s %*s %*s %*s %s\n", &start, &end, name);
    p = strtok(NULL, "\n");
    if (rv == 2) {
      m = &mmap[nmm++];
      m->start = start;
      m->end = end;
      strcpy(m->name, MEMORY_ONLY);
      continue;
    }
    /* search backward for other mapping with same name */
    for (i = nmm-1; i >= 0; i--) {
      m = &mmap[i];
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
      m = &mmap[nmm++];
      m->start = start;
      m->end = end;
      strcpy(m->name, name);
    }
  }
  *nmmp = nmm;
  return 0;
}

struct symlist {
  Elf32_Sym *sym;       /* symbols */
  char *str;            /* symbol strings */
  unsigned num;         /* number of symbols */
};
struct symtab {
  symlist *st;          /* "static" symbols */
  symlist *dyn;         /* dynamic symbols */
};
typedef struct symtab *symtab_t;

static int find_libc(char *name, int len, unsigned long *start,
              mm *mmap, int nmm) {
  int i;
  mm *m;
  char *p;

  for (i = 0, m = mmap; i < nmm; i++, m++) {
    if (!strcmp(m->name, MEMORY_ONLY))
      continue;
    p = strrchr(m->name, '/');
    if (!p) continue;
    p++;
    if (strncmp("libc", p, 4)) continue;
    p += 4;
    /* here comes our crude test -> 'libc.so' or 'libc-[0-9]' */
    if (!strncmp(".so", p, 3) || (p[0] == '-' && isdigit(p[1])))
      break;
  }

  // No found
  if (i >= nmm)  return -1;
  *start = m->start;
  strncpy(name, m->name, len);
  if (strlen(m->name) >= len)
  name[len-1] = '\0';
  return 0;
}

static symlist *
get_syms(int fd, Elf32_Shdr *symh, Elf32_Shdr *strh) {
  symlist *ret = NULL;
  int rv = -1;
  symlist* sl = (symlist *)malloc(sizeof(symlist));
  sl->str = NULL;
  sl->sym = NULL;
  /* sanity */
  if (symh->sh_size % sizeof(Elf32_Sym)) {
    fprintf(stderr, "elf_error\n");
    goto out;
  }
  /* symbol table */
  sl->num = symh->sh_size / sizeof(Elf32_Sym);
  sl->sym = (Elf32_Sym *)malloc(symh->sh_size);
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
  sl->str = (char *)malloc(strh->sh_size);
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

static int do_load(int fd, symtab_t sym) {
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
  shdr = (Elf32_Shdr *) malloc(size);
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
  shstrtab = (char *) malloc(size);
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

static symtab_t load_symtab(char *filename) {
  symtab_t sym = (symtab_t)malloc(sizeof(symtab));
  memset(sym, 0, sizeof(symtab));
  int fd = open(filename, O_RDONLY);
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

static int
lookup2(struct symlist *sl, unsigned char type,
        char *name, unsigned long *val) {
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

static int
lookup_sym(symtab_t s, unsigned char type,
           char *name, unsigned long *val) {
  if (s->dyn && !lookup2(s->dyn, type, name, val))
    return 0;
  if (s->st && !lookup2(s->st, type, name, val))
    return 0;
  return -1;
}

static int
lookup_func_sym(symtab_t s, char *name, unsigned long *val) {
  return lookup_sym(s, STT_FUNC, name, val);
}

/* Find the address of do_dlopen. */
Dyninst::Address Injector::find_do_dlopen() {

  /* Step 1: Load all modules that are currently loaded */
  Dyninst::Address addr = 0;
  mm mmap[MAX_MM];
  int nmm = -1;
  if (0 > load_memmap(mmap, &nmm, pid_)) {
    fprintf(stderr, "ERROR: cannot read memory map\n");
    exit(0);
  }

  /* Step 2: Find the exact name of libc, e.g., libc-2.5.so */
  ulong ldaddr;
  char libc[MAX_LEN];
  if (0 > find_libc(libc, sizeof(libc), &ldaddr, mmap, nmm)) {
    fprintf(stderr, "ERROR: cannot find libc.so\n");
    exit(0);
  }

  /* Step 3: read symbol table */
  symtab_t s = load_symtab(libc);
  if (!s) {
    fprintf(stderr, "ERROR: cannot read symbol table\n");
    exit(0);
  }

  /* Step 4: find do_dlopen */
  if (0 > lookup_func_sym(s, "do_dlopen", &addr)) {
    fprintf(stderr, "ERROR: cannot find do_dlopen\n");
    exit(0);
  }
  fprintf(stderr, "relative addr: %x, libc: %x, verify:%x\n", addr, ldaddr, addr + ldaddr);

  addr += ldaddr;
  fprintf(stderr, "absolute addr: %x\n", addr);
  return addr;
}

static char dlopen_code[] = {
  0x50,                       /* pushl %eax */
  0x68, 0x0, 0x0, 0x0, 0x0,   /* pushl args */
  0xe8, 0x0, 0x0, 0x0, 0x0,   /* call do_dlopen */
  0x58,                       /* popl %eax */
  0x58,                       /* popl %eax */
  0xc3
};
enum {
  OFF_ARGS = 2,
  OFF_DLOPEN = 7,
  OFF_DLRET = 11
};

size_t Injector::get_code_tmpl_size() {
  return sizeof(dlopen_code);
}

char* Injector::get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                              Dyninst::Address code_addr) {
  unsigned long* p = (unsigned long*)&dlopen_code[OFF_DLOPEN];
  *p = (unsigned long)do_dlopen - (unsigned long)(code_addr + OFF_DLRET);
  fprintf(stderr, "do_dlopen: %x, *p:%x\n", do_dlopen, *p);
  p = (unsigned long*)&dlopen_code[OFF_ARGS];
  *p = (unsigned long)args_addr;
  return dlopen_code;
}
