#pragma once

#include <memory>
#include <string>
#include "Context.h"
#include "ast/FunctionDefinitionNode.h"
#include "ast/types/VariableType.h"


void createSystemCall(std::unique_ptr<Context> &context, std::string functionName,
                      std::vector<FunctionArgument> functionparams, std::shared_ptr<VariableType> returnType = nullptr);

void createPrintfCall(const std::unique_ptr<Context> &context);
