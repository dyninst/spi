#ifndef _SPINC_H_
#define _SPINC_H_

/* This is used by user agent implementation. */
#include "agent/agent.h"
#include "agent/event.h"
#include "agent/point.h"
#include "agent/parser.h"
#include "agent/object.h"
#include "agent/snippet.h"
#include "agent/payload.h"
#include "agent/context.h"
#include "agent/propeller.h"
#include "injector/injector.h"

#define AGENT_INIT __attribute__((constructor))
#define AGENT_FINI __attribute__((destructor))

#endif /* _SPINC_H_ */
