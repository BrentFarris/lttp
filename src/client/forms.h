#ifndef LTTP_CLIENT_FORMS_H
#define LTTP_CLIENT_FORMS_H

#include "net.h"
#include "lttp.h"
#include "lttp_form.h"

int Forms_handler(struct lttp* lttp, struct NetHandle* client, void* state, struct lttpForm* form);

#endif