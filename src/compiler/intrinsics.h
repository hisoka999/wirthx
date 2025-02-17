#pragma once

#include <memory>
#include <string>
#include "Context.h"
#include "ast/FunctionDefinitionNode.h"
#include "ast/types/VariableType.h"


void createSystemCall(std::unique_ptr<Context> &context, std::string functionName,
                      std::vector<FunctionArgument> functionparams, std::shared_ptr<VariableType> returnType = nullptr);

void createPrintfCall(const std::unique_ptr<Context> &context);

void createAssignCall(std::unique_ptr<Context> &context);
void createReadLnCall(std::unique_ptr<Context> &context);
void createCloseFileCall(std::unique_ptr<Context> &context);
