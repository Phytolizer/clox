//
// Created by kyle on 8/15/21.
//

#ifndef COMPILER_H_
#define COMPILER_H_

#include "attributes.h"
#include "object.h"
#include "vm.h"

bool compile(const char source[static 1], Chunk* chunk) ATTR_NONNULL(2);

#endif    // COMPILER_H_
