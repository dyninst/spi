<h3 style="text-align: center;">Paradyn Parallel Performance Tools</h3>

<h1>Self-propelled Instrumentation <br><br>
Developer’s Manual
</h1>

<h3>1.0b Release</h3>
<h3>September 2012</h3>


<h5>Computer Science Department
<br>University of Wisconsin–Madison
<br>Madison, WI 53711</h5>

<h5>Computer Science Department
<br>University of Maryland
<br>College Park, MD 20742</h5>

  ------- --------------------
  Email   `bugs@dyninst.org`
  
  Web     `www.dyinst.org`
  ------- --------------------

![image](paradyn_logo)

Introduction
============

Self-propelled instrumentation is a binary instrumentation technique
that dynamically injects a fragment of code into an application process.
The instrumentation is inserted ahead of the control flow within the
process and is propagated into other processes, following communication
events, crossing host boundaries, and collecting function-level trace
data of the execution.

Self-propelled instrumentation contains two major components,
[*Agent*]{} and [*Injector*]{}. [*Agent*]{} is a shared library that
automatically inserts and propagates a piece of payload code at function
call events in a running process, where the payload code contains
user-defined logic, such as generating trace data to files for later
inspection. The instrumentation would propel itself within the process
by following control flow and across thread boundaries, process
boundaries, or even host boundaries by following communication flow.
[*Injector*]{} is a process that causes an application process to load
the Agent shared library, where the Injector should have at least the
same privilege as the application process. Self-propelled
instrumentation does binary instrumentation within the application
process’s address space, avoiding use of the debugging interfaces (e.g.,
Linux ptrace and Windows debug interface) and costly interprocess
communications. Therefore, self-propelled instrumentation does not add
significant overhead to a process during runtime.

Self-propelled instrumentation can be used in many applications that
require low overhead instrumentation and full automation of
instrumentation propagation following control flow. For example, we have
used self-propelled instrumentation for problem diagnosis in distributed
systems [2] and for automated diagram construction for
complex software systems in security analysis [1].

Abstraction
===========

Self-propelled instrumentation has two major components, *Agent*
that is a shared library injected into an application process’s address
space, and *Injector* that injects *Agent*. The following
subsections describe the lower level components in Agent and Injector in
details.

Agent
-----

-   **Agent**. It manages the configuration and does instrumentation. An
    Agent instance is created in the init function of the *Agent*
    shared library.

-   **Event**. It specifies when the initial instrumentation should be
    done after the *Agent* shared library is loaded. Currently,
    there are four types of Event: 1) instrumenting all callees in
    *main* function right away; 2) instrumenting all callees of
    specified functions right away; 3) instrumenting specified function
    calls right away; 4) instrumenting all callees in *main* after a
    given amount of time.

-   **Payload function**. It contains user-specified code. From user’s
    perspective, a payload function will be invoke before or after each
    function call in the process. There are two types of payload
    functions, *entry payload* that is invoked before each function
    call and *exit payload* that is invoked after each function
    call.

-   **Point**. It represents an instrumentation point at current
    function call and is used in Payload function.

-   **Control Flow Graph (CFG) structures**. CFG structures include
    Object, Function, Block, and Edge. An Object represents a binary
    file (i.e., an executable or a shared library), and contains a set
    of functions. A Function contains a set of Blocks. A Block is a
    basic block. An Edge connects two Blocks. Users can get related CFG
    structures of current function call from Point.

<!-- -->

-   **AddressSpace**. It represents the address space of the process. It
    contains a set of Objects in the process. Also, it implements some
    memory management primitives used by the instrumentation engine.

-   **Parser**. It represents a binary code parser that parses binary
    code into structural CFG structures, i.e., Object, Function, Block,
    and Edge.

-   **Propeller**. It manages intra-process instrumentation propagation,
    where it finds function call Points inside current function and uses
    Instrumenter to insert Snippets at these points.

-   **Snippet**. It represents a patch area that contains function calls
    to the Payload function and the relocated function call or the
    relocated call block.

-   **Instrumenter**. It is the instrumentation engine that uses a set
    of Instrumentation Workers to insert Snippets to function call
    points.

-   **Instrumentation Worker**. It represents a mechanism of installing
    instrumentation. Currently, four types of Instrumentation Workers
    are implemented: 1) relocating original function call
    instruction; 2) relocating original call block; 3) relocating nearby
    large springboard block; 4) using trap instruction.

-   **IpcMgr**. It manages inter-process instrumentation propagation by
    creating Channels and using IPC Workers.

-   **Channel**. It represents a unidirectional communication channel,
    containing local process name and remote process name.

-   **IPC Worker**. It implements inter-process instrumentation
    propagation for a particular IPC mechanism (e.g., TCP, UDP, pipe).

Injector
--------

Injector is provided as a command. There are two types of injections.
One is to inject the [*Agent*]{} shared library at the very beginning of
a process. The other is to inject the [*Agent*]{} in the middle of a
running process.

The first type of Injector relies on dynamic linker (i.e., setting the
environment variable LD\_PRELOAD to the path of an [*Agent*]{} shared
library). The second type uses ProcControlAPI to force an application
process to invoke functions in the dlopen family.

How it works
============

This section describes how self-propelled instrumentation works from
user’s perspective. Each subsection is a major step in the workflow.

Building Agent
--------------

Users build their own [*Agent*]{} shared library using self-propelled
instrumentation’s API.

1.  Coding. Users need to write two pieces of code: 1) payload function;
    2) configuration code that registers payload function and does some
    customization and configuration. The configuration code must be
    executed right away when the [*Agent*]{} shared library is loaded
    into the application process, so the configuration code should be in
    the init function of the [*Agent*]{} shared library, i.e., the
    function with gcc directive \_\_attribute\_\_((constructor)).

2.  Building. Users build the code into an [*Agent*]{} shared library
    linking with [*libagent.so*]{} provided by the self-propelled
    instrumentation infrastructure.

Injection
---------

Users run [*Injector*]{} in command line. They specify in command line
arguments the path of an [*Agent*]{} shared library and the application
process to inject to.

One trick to check whether the [*Agent*]{} shared library is injected
successfully is to look at memory maps file of the application process,
i.e., /proc/PID/maps.

Initialization
--------------

The initialization code is executed right away when [*Agent*]{} shared
library is loaded into the application process. It tells self-propelled
instrumentation what are payload functions provided by users, how would
initial instrumentation be done, whether or not to enable inter-process
instrumentation propagation etc.

Initial Instrumentation
-----------------------

Once the configuration code in the [*Agent shared library*]{} finishes
execution inside the application process, the initial instrumentation
would be performed when certain event is triggered (e.g., after 5
seconds) or be performed right away (e.g., instrumenting all function
calls inside the [*main*]{} function).

Instrumentation Propagation
---------------------------

When the initial instrumentation gets executed, then instrumentation
propagates itself either within the process by following control flow,
or across process boundaries by following communication flow.

### Intra-process propagation

![Intra-process Self-propelled Instrumentation
Workflow[]{data-label="fig:intrainst"}](../user/figure/intraprocess.eps){width="90.00000%"}

Figure \[fig:intrainst\] shows an example of intra-process
instrumentation propagation from step 3 to step 5.

In the example, a wrapper function [*instrument*]{} is invoked right
before an original function call [*foo*]{}. The funciton
[*instrument*]{} relies on the instrumentation engine in the agent
library to execute user-provided payload function and to propagate
instrumentation by instrumenting callees inside function [*foo*]{},
which include [*connect*]{} in the example. By this point, an invocation
of [*instrument*]{} is inserted before [*connect*]{}. In this way,
instrumentation propagates from a caller to its callees.

### Inter-process propagation

![Inter-process Self-propelled Instrumentation
Workflow[]{data-label="fig:interinst"}](../user/figure/interprocess.eps){width="90.00000%"}

For inter-process instrumentation propagation, the instrumentation
engine identifies communication initiation functions like [*connect*]{},
[*send*]{}, or [*write*]{} and figures out the remote host. The
instrumentation engine then contacts to the SPI daemon of the remote
host on the other end of communication, which injects the agent shared
library automatically into the target process. Then the intra-process
propagation is performed inside the remote process The workflow is
visualized in Figure \[fig:interinst\]

Examples
========

To illustrate the ideas of Self-propelled instrumentation, we present
some simple code examples that demonstrate how the API can be used.

Writing Payload
---------------

The primary task for users using self-propelled instrumentation is write
payload functions that are invoked before or after each function call.

``` {caption="Writing" payload="" functions=""}
// Entry payload function that is invoked before each function call
void entry_payload(SpPoint* pt) {
  // Get function from point
  SpFunction* func = sp::Callee(pt);
  if (func == NULL) return;

  // If we encounter the system call exit, then print its argument
  if (func->name().compare("exit") == 0) {
    // Get argument
    sp::ArgumentHandle h;
    int* exit_code = (int*)sp::PopArgument(pt, &h, sizeof(int));
    printf("Exit Code: %d\n", *exit_code);
  }
  sp::Propel(pt);
}

// Exit payload function that is invoked after each function call
void exit_payload(SpPoint* pt) {
  // Get function from point
  SpFunction* func = sp::Callee(pt);
  if (func == NULL) return;

  // If we encounter the system call fork, then print its return value
  if (func->name().compare("fork") == 0) {
    pid_t child_pid = sp::ReturnValue(pt);
    // Print child process id from parent process side
    if (child_pid > 0) {
      printf("Child process id: %d\n", child_pid);
    }
  }
}
```

In the above code, we illustrate some common operations that users may
want to do in entry payload function or exit payload function. Users can
get CFG structures (e.g., function, block ...) from the argument [
*pt*]{}. Typically, there are some conditional branches to handle
different functions, e.g., handling exit and fork in the above code.
Users can also get arguments or return value for current function call.

Configuration
-------------

Users also need to provide the configuration code in the constructor
function of the Agent shared library, which is mainly to register the
user-provided payload functions.

``` {caption="Configuration" code=""}
__attribute__((constructor))
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  // Register entry payload function
  agent->SetInitEntry("entry_payload");
  // Register exit payload function
  agent->SetInitExit("exit_payload");
  // Initiate instrumentation
  agent->Go();
}
```

The above code shows the minimum operations needed to configure
self-propelled instrumentation. The major things to do in the
configuration code are to create an Agent object that manages
configurations, registers payload functions, and initiates
instrumentation.

Users may also wish to call ```UseDefaultLibrariesNotToInstrument()``` or ```SetLibrariesNotToInstrument(libs_not_to_inst)``` to speed up parsing at the start of instrumentation.

Using Injector
--------------

The injector can be used to inject the agent shared library into a
process. It can also be injected to monitor all the processes running on
a specific port. The injector executable can be found inside
*$SP\_DIR/\$PLATFORM*. To instrument a particular process with process
id $pid$, use the following command

./injector pid *pid* LIB\_NAME

Similarly, to instrument all the processes running on a specific port,
use the following command

./injector port *port* LIB\_NAME

Extending SPI
-------------

This subsection discusses some code examples to extend self-propelled
instrumentation.

### Parser

``` {caption="Use" customized="" parser=""}
// Extend SpParser and override the Parse() method to customize the parsing logic
class  MyParser : public SpParser {
  public:
    typedef SHARED_PTR(MyParser) ptr;
    AGENT_EXPORT static ptr Create() { return ptr(new MyParser); }
    AGENT_EXPORT virtual PatchAPI::PatchMgrPtr Parse() {
      // Implement your own parsing logic
    }
};

// Register the customized parser to self-propelled instrumentation
__attribute__((constructor))
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  MyParser::ptr parser = MyParser::Create();
  agent->SetParser(parser);
  ...
}
```

The above code first extends SpParser to implement a customized parser
for parsing binary code into CFG structures. Developers mainly need to
implement the *Parse* method. Next, we register the customized
parser in the agent library by using *SpAgent::SetParser* method.

### Event

``` {caption="Use" customized="" event=""}
// Extend SpEvent and override the RegisterEvent() method to do
// initial instrumentation
class  MyEvent : public SpEvent {
  public:
    typedef SHARED_PTR(MyEvent) ptr;
    static ptr Create() { return ptr(new MyEvent()); }
    virtual void RegisterEvent() {}
};

// Register the customized event to self-propelled instrumentation
__attribute__((constructor))
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  MyEvent::ptr event = MyEvent::Create();
  agent->SetInitEvent(event);
  ...
}
```

The above code first extends SpEvent to implement the
*RegisterEvent* method that does initial instrumentation. Next, we
register the customized event in the agent library by using
*SpAgent::SetInitEvent* method.

### Instrumentation Workers

``` {caption="Use" customized="" instrumentation="" worker=""}
// Extend InstWorkerDelegate to build a customized instrumentation worker
class MyInstWorker : public InstWorkerDelegate {
  public:
                // Instrument a point
    virtual bool run(SpPoint* pt) {}

                // Uninstrument a point
                virtual bool undo(SpPoint* pt) {}

                // Save code that will be modified for a point
                virtual bool save(SpPoint* pt) {}

                // How to install instrumentation?
                virtual InstallMethod install_method() {}
};

// Register MyInstWorker in the instrumenter
SpInstrumenter::SpInstrumenter(ph::AddrSpace* as) {
  ...
        workers_.push_back(new SpringboardWorker);
        workers_.push_back(new TrapWorker);
        workers_.push_back(new MyInstWorker);
  ...
}
```

### IPC Workers

``` {caption="Use" customized="" IPC="" worker=""}
class  MyIpcWorker : public SpIpcWorkerDelegate {
  public:
    virtual void SetRemoteStartTracing(char yes_or_no,
                                       SpChannel* c) {}
    virtual void SetLocalStartTracing(char yes_or_no) {}
    virtual char CanStartTracing(int fd) {}
    virtual bool Inject(SpChannel*, char* agent_path = NULL) {}
    virtual SpChannel* GetChannel(int fd,
                                  ChannelRW rw,
                                  void* arg = NULL) {}
    virtual void CloseChannel(int fd) {}
};

// Register MyIpcWorker to self-propelled instrumentation
SpIpcMgr::SpIpcMgr() {
  ...
  pipe_worker_ = new SpPipeWorker;
  worker_set_.insert(pipe_worker_);

  tcp_worker_ = new SpTcpWorker;
  worker_set_.insert(tcp_worker_);

  my_ipc_worker_ = new MyIpcWorker;
  worker_set_.insert(my_ipc_worker_);
  ...
}
```

Class Reference
===============

Class Agent
-----------

**Declared in**: src/agent/agent.h

`void SetInitEntry(string funcd_name);`

Sets the entry payload function. The payload function should be linked
to the agent library or should be in the agent library. By default, the
entry payload function is “default\_entry”, which simply prints out all
executed functions’ name.

`void SetInitExit(string func_name);`

Sets the exit payload function. By default, there is not exit payload
function.

`void SetLibrariesNotToInstrument(const StringSet& libs);`

Sets the set of names of libraries loaded in the application process
that should not be instrumented. By default, we instrument both the executable
binary code and exported functions from loaded shared libraries.

`void UseDefaultLibrariesNotToInstrument();`

Excludes a default list of libraries from being instrumented to speed up parsing, including common loaded shared libraries and exported functions related to Dyninst.

`void SetFuncsNotToInstrument(const StringSet& funcs);`

Sets the set of names of functions in the application process that will
NOT be instrumented. By default, we instrument all functions.

`void EnableParseOnly(const bool yes_or_no);`

Enables or disables ParseOnly option. If yes\_or\_no is true, then after
parsing the binary code, we don’t do instrumentation; otherwise,
instrumentation is conducted after parsing. By default, ParseOnly option
is disabled.

`void EnableIpc(const bool yes_or_no);`

Enables or disables inter-process instrumentation propagation. By
default, inter-process instrumentation propagation is disabled.

`void EnableHandleDlopen(const bool yes_or_no);`

Enables or disables propagating instrumentation to library loaded via
dlopen. By default, it is disabled.

`void EnableMultithread(const bool yes_or_no);`

Enables or disables propagating instrumentation across thread. By
default, it is disabled.

`void Go();`

Starts initial instrumentation.

Class PointCallHandle
---------------------
**Declared in:** src/agent/payload.h

`SpPoint* GetPoint();`

Get the SpPoint reference associated with this point call instance.

`SpFunction* GetCallee();`

Get the callee.

`SpFunction* void* GetUserInfo();`

Get the UserInfo that’s returned by the payload entry function. This should only
get called in the payload exit function.

`long GetReturnValue();`

Get the return value of the call. This should only get called in the payload exit
function.


Class SpPoint
-------------

**inherits:** PatchAPI::Point

**Declared in**: src/agent/patchapi/point.h

`bool tailcall();`

Indicates whether or not the function call at this point is a tail call.

`SpBlock* GetBlock() const;`

Returns the call block at this point.

`SpObject* GetObject() const;`

Returns the binary object at this point.

Class SpObject
--------------

**inherits:** PatchAPI::PatchObject

**Declared in**: src/agent/patchapi/object.h

`std::string name() const;`

Returns the binary object’s name.

This class inherits PatchAPI::PatchObject. Please refer to PatchAPI
document for the complete list of methods.

Class SpFunction
----------------

**inherits:** PatchAPI::PatchFunction

**Declared in**: src/agent/patchapi/cfg.h

`SpObject* GetObject() const;`

Returns the binary object containing this function.

`std::string GetMangledName();`

Returns the mangled name of this function.

`std::string GetPrettyName();`

Returns the demangled name of this function.

`std::string name();`

Returns the demangled name of this function, the same as calling
GetPrettyName().

This class inherits PatchAPI::PatchFunction. Please refer to PatchAPI
document for the complete list of methods.

Class SpBlock
-------------

**inherits:** PatchAPI::PatchBlock

**Declared in**: src/agent/patchapi/cfg.h

`SpObject* GetObject() const;`

Returns the binary object containing this block.

`in::Instruction::Ptr orig_call_insn() const;`

Returns an InstructionAPI::Instruction instance of the call instruction
in this block.

This class inherits PatchAPI::PatchBlock. Please refer to PatchAPI
document for the complete list of methods.

Class SpEdge
------------

**inherits:** PatchAPI::PatchEdge

**Declared in**: src/agent/patchapi/cfg.h

This class inherits PatchAPI::PatchEdge. Please refer to PatchAPI
document for the complete list of methods.

Utility Functions
-----------------

**Declared in**: src/agent/payload.h

Utility functions are used when writing payload functions.

`SpFunction* Callee(SpPoint* pt);`

Returns an instance of SpFunction for current function call. The
parameter *pt* represents the instrumentation point for current function
call. If it fails (e.g., cannot find a function), it returns NULL.

`bool IsInstrumentable(SpPoint* pt);`

Indicates whether or not the point is instrumentable.

`void Propel(SpPoint* pt);`

Propagates instrumentation to callees of the function called at the
specified point *pt*.

`bool IsIpcWrite(SpPoint* pt); bool IsIpcRead(SpPoint* pt);`

Indicates whether or not the function called at the specified point is a
inter-process communication write / read function.

`struct ArgumentHandle; void* PopArgument(SpPoint* pt, ArgumentHandle*
h, size_t size);`

Gets a pointer to an parameter of the function call at the specified
point *pt*. All parameters passed to the function call are in a
stack associated with the ArgumentHandle structure *h*. Leftmost
parameter is at the top of the stack. The *size* parameter specifies
the size of the parameter that is about to be popped.

`long ReturnValue(SpPoint* pt);`

Returns the return value of the function call at the specified point
*pt*.

Class SpInjector
----------------

**Declared in**: src/injector/injector.h

`static ptr Create(Dyninst::PID pid);`

Creates a Boost shared pointer pointing to a SpInjector instance. The
parameter *pid* specifies the pid of the remote process that will be
injected.

`bool Inject(const char* lib_name);`

Injects a library with *lib\_name*.

Class SpParser
--------------

**Declared in**: src/agent/parser.h

`static ptr Create();`

Creates a Boost shared pointer pointing to a SpParser instance.

`SpObject* exe() const;`

Returns the executable’s binary object.

`PatchAPI::PatchMgrPtr Parse();`

The major parsing method, which builds CFG data structures to assist
instrumentation.

`string agent_name()`

Returns the agent library’s full path name.

`bool injected()`

Indicates whether or not this agent library is injected in the middle of
running current process.

`void GetFrame(long* pc, long* sp, long* bp)`

Get the values of three registers (pc, sp, and bp) of the call frame
where the agent is injected. The three parameters are output parameters.

`SpFunction* FindFunction(Dyninst::Address absolute_addr);`

Returns the function that contains the address *absolute\_addr*. It
returns NULL if no function is found.

`SpFunction* FindFunction(string mangled_name);`

Returns one of the functions that have the mangled function name. It
returns NULL if no function is found.

`bool FindFunction(string func_name, FuncSet* found_funcs);`

Get all the functions that have the demangled function name.

`SpFunction* callee(SpPoint* point, bool parse_indirect = false);`

Returns the callee at the given call *point*. If
*parse\_indirect* is false and the call instruction is an indirect
call instruction, then we skip parsing it and return NULL immediately.
If the callee is not found, NULL is returned.

`string DumpInsns(void* addr, size_t size);`

Returns a nice textual representation of all instructions at the memory
area of \[ *addr*, *addr* + size\].

`static bool ParseDlExit(SpPoint* pt);`

Detects the dlopen function call, and parses the library that is just
loaded. It is invoked immediately after the dlopen function call is
returned.

Class SpContext
---------------

**Declared in**: src/agent/context.h

`static SpContext* Create();`

Creates a Boost shared pointer pointing to a SpContext instance.

`string init_entry_name() const`

Returns initial entry payload function’s demangled name.

`string init_exit_name() const`

Returns initial exit payload function’s demangled name.

`typedef std::set<SpFunction*> FuncSet;`

`void GetCallStack(FuncSet* func_set);`

Get a set of functions in the call stack, when the agent library is
injected. *func\_set* is the output parameter.

`bool IsMultithreadEnabled() const;`

Indicates whether instrumentation propagation across thread boundary is
supported.

`bool IsHandleDlopenEnabled() const;`

Indicates whether parsing newly loaded shared library is supported.

`bool IsDirectcallOnlyEnabled() const;`

Indicates whether or not self-propelled instrumentation only instruments
direct function call.

`SpPropeller::ptr init_propeller() const;`

Returns an instance of SpPropeller.

`typedef void* PayloadFunc; PayloadFunc init_entry() const; PayloadFunc
init_exit() const;`

Returns the start address of user-provided entry payload function or
exit payload function.

`PayloadFunc wrapper_exit() const; PayloadFunc wrapper_entry() const;`

Returns the start address of the wrapper of entry payload function or
exit payload function.

Events
------

**Declared in**: src/agent/event.h

This is a collection of events that trigger initial instrumentation.

**Class SpEvent**

It is the base class of all event classes. This event does not trigger
any instrumentation.

`static ptr Create();`

Creates a Boost shared pointer pointing to a SpEvent instance.

`virtual void RegisterEvent();`

It registers the event to agent. This function is called in Agent::Go().

**Class AsyncEvent**

It is the asynchronous event that will happen when specified signal
occurs.

`static ptr Create(int signum = SIGALRM, int sec = 5);`

Creates a Boost shared pointer pointing to an AsyncEvent instance.

The parameter *signum* specifies the signal number, and the
parameter *sec* is only effective when signum is SIGALRM.

**Class SyncEvent**

It is the synchronous event that will do instrumentation immediately. If
the agent library is loaded via LD\_PRELOAD, then it instruments main()
function immediately. If the agent library is injected via injector,
then it instruments all functions in the call stack when the agent
library is loaded.

`static ptr Create();`

Creates a Boost shared pointer pointing to a SyncEvent instance.

**Class FuncEvent**

It instruments all of the specified functions immediately.

`static ptr Create(StringSet& funcs);`

Creates a Boost shared pointer pointing to a FuncEvent instance. The
parameter [*funcs*]{} specifies a set of demangled names of functions
that are going to be instrumented.

**Class CallEvent**

It installs instrumentation at all call sites of specified functions
immediately.

`static ptr Create(StringSet& funcs);`

Creates a Boost shared pointer pointing to a CallEvent instance. The
parameter [*funcs*]{} specifies a set of demangled names of function
calls that are going to be instrumented.

**Class CombEvent**

It is a compound event that can combine all the above events.

`static ptr Create(EventSet& events);`

Creates a Boost shared pointer pointing to a CombEvent instance. The
parameter [*events*]{} specifies a set of events.

Class SpPropeller
-----------------

**Declared in**: src/agent/propeller.h

`static ptr Create();`

Creates a Boost shared pointer pointing to a SpPropeller instance.

`bool go(SpFunction* func, PayloadFunc entry, PayloadFunc exit,
SpPoint* pt = NULL, StringSet* inst_calls = NULL);`

It propagates instrumentation to callees of specified function
*func*. The parameters *entry* and *exit* specify the
payload functions to be installed for those callees. The parameter
*point* is the point where *func* is invoked. The parameter
*inst\_calls* specifies all function calls that need to be
instrumented, which is used only in CallEvent.

Class SpSnippet
---------------

**Declared in**: src/agent/snippet.h

`char* BuildBlob(const size_t est_size, const bool reloc = false);`

Returns a pointer to a blob of code snippet, which contains relocated
code and invocations to payload functions. The parameter *est\_size*
specifies the estimate size to allocate the buffer for the code snippet.
The parameter *reloc* indicates whether or not we need to relocate a
call block.

`size_t GetBlobSize() const;`

Returns the code snippet’s size.

`SpBlock* FindSpringboard();`

Returns a springboard block.

`char* RelocateSpring(SpBlock* spring_blk);`

Relocates the basic block *spring_blk* that is used as springboard
block, and returns the buffer containing the relocated springboard
block.

`size_t GetRelocSpringSize() const;`

Returns the relocated springboard block size.

`Dyninst::Address GetSavedReg(Dyninst::MachRegister reg);`

Returns the value of a saved register *reg*.

`long GetRetVal();`

Returns the return value of current function call.

`void* PopArgument(ArgumentHandle* h, size_t size);`

Pops an parameter of current function call.

`PayloadFunc entry() const;`

Returns entry payload function’s address.

`PayloadFunc exit() const;`

Returns exit payload function’s address.

`SpPoint* point() const;`

Returns the function call point where this code snippet is installed.

Internals
=========

This section describes the detailed internal design of self-propelled
instrumentation, from developer’s perspective. Current implementation is
on Linux.

Injector
--------

The main injection procedure has two main steps:

-   Force an application process to execute [*do\_dlopen*]{} or [
    *\_\_libc\_dlopen\_mode*]{} functions in libc.so to load shared
    library libijagent.so. We use ProccontrolAPI’s IRPC mechanism to
    remotely invoke functions.

-   Force an application process to execute [*ij\_agent*]{} function in
    libijagent.so to load the user-specified shared library. In this
    step, we have to pass parameters from injector to the application
    process by IPC mechanism, where we use shared memory in current
    implementation and the shared memory id is 1986. If the application
    also notifies the injector via shared memory whether the library
    loading is successful.

Here are some design rationales:

**Why don’t we directly use do\_dlopen or \_\_libc\_dlopen\_mode to load
user-specified shared library?** Because do\_dlopen is not a public
function, which is unsafe to use. Oftentimes, do\_dlopen causes the
application process to crash, e.g., the library is not found. On the
other hand, dlopen is safe to use, which would not causes the
application process to crash.

**To follow up the above question, since dlopen is safe to use, why
don’t we directly call dlopen to load user-specified library?** We are
not allowed to do so. libc.so has sanity check on calling dlopen. If we
call dlopen using IRPC, dlopen would fail, although it won’t crash
injectee process. In sum, the benefit of providing a level of
indirection is, we can use unsafe do\_dlopen function to load a
controlled library libijagent.so, from libijagent.so, we use safe dlopen
function to load uncontrolled user-provided library.

**Why sometimes we may use \_\_libc\_dlopen\_mode?** Because ... if libc
is stripped, then we cannot find do\_open, but we can still find
\_\_libc\_dlopen\_mode.

**Why we use IPC mechanism to pass parameters to ij\_agent in
libijagent.so?** ij\_agent is a function that calls dlopen. After dlopen
is invoked, we want to check whether the loading is successful. Even
when dlopen fails, we also want to know the error message. Therefore, we
need to do IPC for error report or checking return value. In this case,
why don’t we have an easy and uniformed way to pass argument and check
return value?

Agent
-----

### Parsing

When Agent library is loaded, it needs to parse the binary code into
PatchAPI CFG (Control Flow Graph) structures that are used in later
instrumentation.

The parsing is done in [*SpParser::Parse*]{}, which consists of these
steps:

-   Parse binary code into SymtabAPI::Symtab objects.

-   Use SymtabAPI::Symtab objects to create SpObject (subclass of
    PatchAPI::PatchObject) objects, which indirectly relies on [
    *ParseAPI::parse*]{} to do the runtime parsing.

### Initial Instrumentation

Initial instrumentation is encapsulated in SpEvent series classes.

In particular, for SyncEvent, we make the semantics to be “do initial
instrumentation right away”. We handle this in two cases. If the agent
library is injected via an injector, we instrument all functions in the
call stack when injection happens, using StackwalkerAPI. If the agent
library is loaded at the beginning of running an application process via
setting LD\_PRELOAD environment variable, then we instrument [*main*]{}
function.

### Intra-process Propagation {#sec:intrainst}

The instrumentation is to transfer control from original binary code in
an application process to a snippet of code that is built by
self-propelled instrumentation. This snippet of code contains these
things in order:

-   Relocated code, which may be a relocated call instruction or a
    relocated call block, depending on what instrumentation worker is
    used.

-   Context saving code, which saves registers in the stack.

-   Code to invoke an entry payload function.

-   Context restoring code, which restores registers from the stack.

-   Code to invoke the original function being instrumented.

-   Context saving code, which saves registers in the stack (OPTIONAL).

-   Code to invoke an exit payload function (OPTIONAL).

-   Context restoring code, which restores registers from the stack
    (OPTIONAL).

-   Code to jump back to the original code.

To install instrumentation to a function call, we try these
instrumentation workers in order:

-   RelocCallInsnWorker: Relocates call instruction and replaces the
    call instruction with a jump instruction. This may fail because a
    jump instruction may be larger than a call instruction.

-   RelocCallBlockWorker: Relocates call block and replaces the call
    block with a jump instruction. This may fail because a jump
    instruction may be larger than a call block.

-   SpringboardWorker: Relocates call block C and replaces the call
    block with a short jump that transfers control to a nearby block as
    springboard block S. Relocates S and replace S with two jump
    instructions, one of which jumps to the relocated S (denoted as S’),
    and the other jumps to the snippet. To sum up, C -&gt; S -&gt; S’
    -&gt; S -&gt; snippet -&gt; C. This may fail because a nearby
    springboard block that is large enough to place two long jumps may
    not be found.

-   TrapWorker: Relocates call instruction and replaces it with a trap
    instruction.

### Inter-process Propagation

Inter-process propagation is managed by SpIpcMgr class, which relies on
IPC (inter-process communication) workers to handle different IPC
mechanisms. The current implementation of inter-process propagation
supports two IPC mechanisms, pipe and tcp.

Generally, inter-process propagation works by following these steps:

-   Process A detects the initialization of communication (e.g., [
    *connect*]{}, [*write*]{}, [*send*]{} ...) and finds the
    approapriate IPC worker for further processing.

-   The IPC worker of Process A creates a write channel that knows the
    Process A is talking to Process B right now. Then, the IPC worker of
    Process A injects the agent library specified by the environment
    variable SP\_AGENT\_DIR to Process B.

-   The IPC worker of Process A synchronizes the message receiving event
    in Process B to the according send event in Process A that triggers
    cross-process propagation. If we don’t do this, then the
    user-defined logic may trace delayed arriving message in Process B
    that are sent before the instrumentation in Process A.

-   For each read event or message receiving event (e.g., [*recv*]{}, [
    *read*]{} ...), Process B needs to check whether or not it can start
    executing the user-defined logic in the payload function, which is
    done by invoking [ *StartTracing*]{} or [*IsIpcRead*]{} in the
    payload function by users.

Different IPC workers have different implementations for detecting the
initialization of communication, injecting agent to remote process, and
synchronizing message receiving event with sending event.

For injecting agent to remote process, the pipe worker directly invoke
injector to inject the agent library to the process on the same machine,
while the tcp worker uses ssh to remotely invoke injector on the remote
machine to find PID of the remote process from port number and to inject
the agent library to the remote process.

For synchronizing message receiving event with send event, the pipe
worker uses shared memory, while the tcp worker uses TCP OOB
(out-of-band) mechanism.

### Inter-thread Propagation

The inter-thread instrumentation propagation is implemented in the
SpThreadMgr class. The basic idea is, when detecting
[*pthread\_create*]{}, we get its first argument, which is the address
for the routine that will be run on a new thread. Next, we find a
function according to this function address, then we instrument the
function’s callees.

### x86\_64 Issues

There are two issues related to x86\_64 specific implementation of
intra-process propagation.

The first issue is we need to pay special attention to the instructions
using RIP register, because when we relocate such instructions, the RIP
value changes accordingly. For details, search “RIP” or “use\_pc” in
src/agent/snippet-x86\_64.cc.

The second issue is that we have a customized memory allocator to
allocate buffer to a snippet. The goal is to bring the snippet as close
to the original call instruction as possible, so that we can generate
smaller jump instruction and increase our chance to use
RelocCallInsnWorker that performs best amongst all instrumentation
workers. The basic idea for the customized memory allocator is as
follows:

-   After parsing, for each object, we bind a huge free memory area
    before the starting address of this object, where we actually use
    [*mmap*]{} to reserve such free memory area.

-   For each mmap-ed memory area, we create a free list for small
    buffers of size 256 bytes, a free list for medium buffers of size
    512 bytes, and a free list for large buffers of size 4096 bytes. All
    small buffers account for 80% of size in each mmap-ed memory area,
    all medium buffers for 10%, and all large buffers for 5%. In this
    way, each object would have three free lists.

-   When invoking SpAddrSpace::malloc, we first get the object that
    contains the call instruction, then we see whether the memory
    allocation can be satisfied in the small buffer free list, the
    medium buffer free list, and the huge buffer free list in order. If
    the memory allocation cannot be satisfied in free lists, then we
    resort to use [*malloc*]{} (on 32-bit Linux, we always use
    [*malloc*]{}).

### Parsing Newly Loaded Library

When detecting a function call to be [*dlopen*]{}, we would parse the
newly loaded library.

### Thread-safe

To ensure operations in the payload function are thread-safe, we
implicitly protect the public interfaces with locks. The implementation
of the lock can be found in src/common/utils.cc.

Installation
============

This appendix describes how to build self-propelled instrumentation from
source code, which can be downloaded from <http://www.paradyn.org> or
<http://www.dyninst.org>.

Before starting to build self-propelled instrumentation, you have to
make sure that you have already installed and built Dyninst (refer
Appendix D in Dyninst Programming Guide http://www.dyninst.org/sites/default/files/manuals/dyninst/dyninstProgGuide.pdf for building Dyninst).

Building self-propelled instrumentation on Linux platforms is a very
simple three step process that involves: unpacking the self-propelled
instrumentation source, configuring and running the build.

Getting the source
------------------

Self-propelled instrumenation‘s source code is packaged in *tar.gz*
format. If your self-propelled instrumentation source tarball is called
*src\_spi.tar.gz*, then you could extract it with the commands *gunzip
src\_spi.tar.gz ; tar -xvf src\_spi.tar*. This will create a list of
directories and files.

Configuration
-------------

After unpacking, the next thing is to set SP\_DIR, DYNINST\_ROOT,
PLATFORM and DYNLINK variables in *config.mk*. DYNINST\_ROOT should be
set to path of the directory that contains subdirectories like
dyninstAPI, parseAPI etc., i.e. within dyninst directory SP\_DIR should
be set to the the path of the current working directory (where
self-propelled instrumentation is installed).

DYNLINK should be set *true* for building agent as a small shared
library that relies on other shared libraries. Otherwise, set DYNLINK as
*false* for buidling a single huge shared library that static-linked all
libraries

PLATFORM should be set to one of the following values depending upon
what operating system you are running on:

  -------------------------- --- ---------------------------------------------------
  i386-unknown-linux 2.4      :  Linux 2.4/2.6 on an Intel x86 processor
  x86\_64-unknown-linux2.4    :  Linux 2.4/2.6 on an AMD-64/Intel x86-64 processor
  -------------------------- --- ---------------------------------------------------

Before building, you should also check whether LD\_LIBRARY\_PATH
environment variable is set. If you are using bash shell, then open
 /.bashrc file and check if LD\_LIBRARY\_PATH is already present. If
not, then LD\_LIBRARY\_PATH variable should be set in a way that it
includes [\$DYNINST\_ROOT]{}/lib directories. If you are using C shell,
then do the above mentioned tasks in  /.cshrc file.

If you want to use inter-process propagation, then you should also set
the environment variable SP\_AGENT\_DIR to be the full path of the agent
shared library that will be injected on every machine involved.

Building
--------

Once config.mk is set, you are ready to build self-propelled
instrumentation. Move to PLATFORM directory and execute the command
*make*. This will build libagent.so, injector, test-cases and
test-programs. Other make commands for custom building are as follows:

-   make spi: build injector and libagent.so.

-   make injector\_exe: build injector.

-   make agent\_lib: build libagent.so.

-   make unittest: build unittests.

-   make mutatee: build simple mutatees.

-   make external\_mutatee: build real world mutatees, including unix
    core utilities and gcc.

-   make test: equivalent to make unittest + make mutatee + make
    external\_mutatee

-   make clean\_test: clean test stuffs.

-   make clean: only clean core self-propelled stuffs, excluding
    dependency.

-   make clean\_all: clean everything, including dependency.

-   make clean\_objs: clean core self-propelled objs.

That is it! Now, you are all set to use Self propelled intrumentation.

Testing and Debugging
=====================

This section covers the testing framework and some debugging tricks of
self-propelled instrumentation.

Testing
-------

We rely on Google C++ Testing Framework for testing, whose source code
is included in self-propelled instrumentation’s project directory, that
is, \$SP\_DIR/src/test/gmock.

There are two types of tests in the project, unit tests and system
tests.

Each source code file is associated with a unit test suite, consisting
of a set of test cases to test component units. The unit test suite code
is located within the same folder of the source code it tests (mostly in
\$SP\_DIR/src/agent, \$SP\_DIR/src/injector, and \$SP\_DIR/src/common),
and the unit test suite file name has a suffix “\_unittest”. For
example, if a source code file name is agent.cc, then its unit test
suite file name is agent\_unittest.cc.

System tests are located in \$SP\_DIR/src/test, which test the complete
system. There are application programs for testing. The application
programs that are written by self-propelled instrumentation developers
are located in \$SP\_DIR/src/test/mutatee. Those that are real external
programs (e.g., unix core utilities) are downloaded by the script in
\$SP\_DIR/scripts/build\_mutatees.sh that is invoked implicitly by
Makefile.

To run tests, the current working directory need to be the building
directory \$SP\_DIR/\$PLATFORM, and we invoke test/\$TEST\_NAME.exe to
run a test. For example, we can execute test/coreutils\_systest.exe to
run the system test for instrumenting unix core utilities, or run
test/injector\_unittest.exe to run the system test for injector.

Debugging
---------

We provide several useful environment variables to assist debugging.

-   SP\_DEBUG: If this environment variable is set, then the execution
    of self-propelled instrumentation would print out detailed debug
    information to the stderr.

-   SP\_FDEBUG: Redirects debug information to output files in /tmp/spi/.

-   SP\_DEBUG\_[OPTION]: Enables debugging information for different components
    of SPI. Options are INJECTOR, COMMON, PATCHAPI, IPC, WORKER, SIGTRAP, AGENT.

-   SP\_TIMING: Enables timestamps on debugging information output

-   SP\_COREDUMP: If this environment variable is set, then it enables
    core dump when segfault happens.

-   SP\_TEST\_RELOCINSN: If this environment variable is set, then
    self-propelled instrumentation only uses instruction relocation
    instrumentation worker

-   SP\_TEST\_RELOCBLK: If this environment variable is set, then
    self-propelled instrumentation only uses call block relocation
    instrumentation worker

-   SP\_TEST\_SPRING: If this environment variable is set, then
    self-propelled instrumentation only uses sprint block
    instrumentation worker

-   SP\_TEST\_TRAP: If this environment variable is set, then
    self-propelled instrumentation only uses trap instrumentation
    worker.

-   SP\_NO\_TAILCALL: If this environment variable is set, then
    self-propelled instrumentation will not instrument tail calls.

-   SP\_LIBC\_MALLOC: If this environment variable is set, then on
    x86\_64, self-propelled instrumentation will always use libc malloc,
    instead of using our customized memory allocator to find nearby
    buffers.

-   SP\_NO\_LIBC\_MALLOC: If this environment variable is set, then on
    x86\_64, self-propelled instrumentation will not use libc malloc
    when no any nearby buffer is found.

Directory Organization
======================

-   common.flag.mk : Compilation flags

-   config.mk : User-defined configuration file

-   spi.target.mk : Major compilation rules

-   test.target.mk : Compilation rules for tests

-   scripts/ : Containing some scripts to assist testing

-   h/ : Public header files

-   src/ : Source code for injector, agent, and tests

    -   agent/ : Source code for agent

    -   common/ : Common code used by injector and agent

    -   injector/ : Source code for injector

    -   test/ : Some tests

-   user\_agent/ : Example user-provided agents

-   x86-unknown-linux-2.4/ : Build directory for x86-64

-   i386-unknown-linux-2.4/ : Build directory for x86

References
==========
[1] W. Fang, B. P. Miller, and J. A. Kupsch. Automated tracing and visualization of
software security structure and properties. In VizSec 2012, Seattle, Washington, USA,
Oct. 2012.

[2] A. V. Mirgorodskiy, N. Maruyama, and B. P. Miller. Problem diagnosis in large-scale
computing environments. In Supercomputing 2006, Tampa, Florida, USA, Nov. 2006.

