#include "VariableAssignmentNode.h"
#include <iostream>
#include "FunctionCallNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"

VariableAssignmentNode::VariableAssignmentNode(const std::string_view variableName,
                                               const std::shared_ptr<ASTNode> &expression) :
    m_variableName(variableName), m_expression(expression)
{
}

void VariableAssignmentNode::print()
{
    std::cout << m_variableName << ":=";
    m_expression->print();
    std::cout << ";\n";
}

llvm::Value *VariableAssignmentNode::codegen(std::unique_ptr<Context> &context)
{

    // Look this variable up in the function.
    llvm::AllocaInst *V = context->NamedAllocations[m_variableName];

    if (!V)
        return LogErrorV("Unknown variable name for assignment: " + m_variableName);


    auto result = m_expression->codegen(context);

    if (V->getAllocatedType()->isIntegerTy() && result->getType()->isIntegerTy())
    {
        auto targetType = llvm::IntegerType::get(*context->TheContext, V->getAllocatedType()->getIntegerBitWidth());
        if (V->getAllocatedType()->getIntegerBitWidth() != result->getType()->getIntegerBitWidth())
        {
            result = context->Builder->CreateIntCast(result, targetType, true, "lhs_cast");
        }

        context->Builder->CreateStore(result, V);
        return result;
    }


    if (V->getAllocatedType()->isStructTy() && result->getType()->isPointerTy())
    {
        auto llvmArgType = V->getAllocatedType();

        auto memcpyCall = llvm::Intrinsic::getDeclaration(
                context->TheModule.get(), llvm::Intrinsic::memcpy,
                {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
        std::vector<llvm::Value *> memcopyArgs;
        // llvm::AllocaInst *alloca = context->Builder->CreateAlloca(llvmArgType, nullptr, m_variableName + "_ptr");

        const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        uint64_t structSize = DL.getTypeAllocSize(llvmArgType);


        memcopyArgs.push_back(context->Builder->CreateBitCast(V, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateBitCast(result, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->getInt64(structSize));
        memcopyArgs.push_back(context->Builder->getFalse());

        context->Builder->CreateCall(memcpyCall, memcopyArgs);

        return result;
    }

    context->Builder->CreateStore(result, V);
    return result;
}
