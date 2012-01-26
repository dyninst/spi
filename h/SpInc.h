#ifndef _SPINC_H_
#define _SPINC_H_

/* This is used by user agent implementation. */
#include "SpAgent.h"
#include "SpEvent.h"
#include "SpPoint.h"
#include "SpUtils.h"
#include "SpParser.h"
#include "SpObject.h"
#include "SpSnippet.h"
#include "SpPayload.h"
#include "SpContext.h"
#include "SpPropeller.h"

#define AGENT_INIT __attribute__((constructor))
#define AGENT_FINI __attribute__((destructor))

#endif /* _SPINC_H_ */
