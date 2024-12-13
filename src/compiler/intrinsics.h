#pragma once

#include <memory>
#include <string>
#include "Context.h"
#include "ast/FunctionDefinitionNode.h"
#include "ast/VariableType.h"


void createSystemCall(std::unique_ptr<Context> &context, std::string functionName,
                      std::vector<FunctionArgument> functionparams, std::shared_ptr<VariableType> returnType = nullptr);

void createPrintfCall(std::unique_ptr<Context> &context);


void writeLnCodegen(std::unique_ptr<Context> &context, size_t length);

void writeLnStrCodegen(std::unique_ptr<Context> &context);
