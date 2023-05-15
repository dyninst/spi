AGENT_NAME=libmyagent.so

AGENT_SRCS+=MyAgent.C fpva.C fpva_utils.C fpva_checker.C trace_mgr.C

AG_IFLAGS+=-I/usr/include/libxml2

AGENT_LDFLAGS+=-lxml2

