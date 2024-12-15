#include "BinaryOperationNode.h"
#include <iostream>
#include "UnitNode.h"
#include "compiler/Context.h"

BinaryOperationNode::BinaryOperationNode(Operator op, const std::shared_ptr<ASTNode> &lhs,
                                         const std::shared_ptr<ASTNode> &rhs) :
    ASTNode(), m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

void BinaryOperationNode::print()
{
    m_lhs->print();
    std::cout << static_cast<char>(m_operator);
    m_rhs->print();
}

llvm::Value *BinaryOperationNode::generateForInteger(llvm::Value *lhs, llvm::Value *rhs,
                                                     std::unique_ptr<Context> &context)
{
    size_t maxBitWith = std::max(lhs->getType()->getIntegerBitWidth(), rhs->getType()->getIntegerBitWidth());
    auto targetType = llvm::IntegerType::get(*context->TheContext, maxBitWith);
    if (maxBitWith != lhs->getType()->getIntegerBitWidth())
    {
        lhs = context->Builder->CreateIntCast(lhs, targetType, true, "lhs_cast");
    }
    if (maxBitWith != rhs->getType()->getIntegerBitWidth())
    {
        rhs = context->Builder->CreateIntCast(rhs, targetType, true, "rhs_cast");
    }


    switch (m_operator)
    {
        case Operator::PLUS:
            return context->Builder->CreateAdd(lhs, rhs, "addtmp");
        case Operator::MINUS:
            return context->Builder->CreateSub(lhs, rhs, "subtmp");
        case Operator::MUL:
            return context->Builder->CreateMul(lhs, rhs, "multmp");
    }
    return nullptr;
}

llvm::Value *BinaryOperationNode::generateForString(llvm::Value *lhs, llvm::Value *rhs,
                                                    std::unique_ptr<Context> &context)
{

    auto varType = VariableType::getString();
    auto valueType = VariableType::getInteger(8)->generateLlvmType(context);
    auto llvmRecordType = varType->generateLlvmType(context);
    auto indexType = VariableType::getInteger(64)->generateLlvmType(context);
    auto stringAlloc = context->Builder->CreateAlloca(llvmRecordType, nullptr, "combined_string");


    auto arrayRefCountOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 0, "combined_string.refCount.offset");
    auto arraySizeOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 1, "combined_string.size.offset");


    auto arrayPointerOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 2, "combined_string.ptr.offset");

    auto lhsIndexPtr = context->Builder->CreateStructGEP(llvmRecordType, lhs, 1, "lhs.size.offset");

    auto rhsIndexPtr = context->Builder->CreateStructGEP(llvmRecordType, rhs, 1, "rhs.size.offset");
    // lhs size
    auto lhsIndex = context->Builder->CreateLoad(indexType, lhsIndexPtr, "lhs.size");
    auto rhsIndex = context->Builder->CreateLoad(indexType, rhsIndexPtr, "rhs.size");

    auto newSize = context->Builder->CreateAdd(lhsIndex, rhsIndex, "new_size");


    // change array size
    context->Builder->CreateStore(context->Builder->getInt64(1), arrayRefCountOffset);
    context->Builder->CreateStore(newSize, arraySizeOffset);
    auto allocCall = context->Builder->CreateCall(context->TheModule->getFunction("malloc"), newSize);

    auto memcpyCall = llvm::Intrinsic::getDeclaration(
            context->TheModule.get(), llvm::Intrinsic::memcpy,
            {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
    {
        auto boundsLhs = context->Builder->CreateGEP(
                valueType, allocCall, llvm::ArrayRef<llvm::Value *>{context->Builder->getInt64(0)}, "", false);
        auto lhsPtrOffset = context->Builder->CreateStructGEP(llvmRecordType, lhs, 2, "lhs.ptr.offset");
        auto loadResult =
                context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), lhsPtrOffset);
        std::vector<llvm::Value *> memcopyArgs;
        memcopyArgs.push_back(context->Builder->CreateBitCast(boundsLhs, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateBitCast(loadResult, context->Builder->getPtrTy()));
        memcopyArgs.push_back(lhsIndex);
        memcopyArgs.push_back(context->Builder->getFalse());

        context->Builder->CreateCall(memcpyCall, memcopyArgs);
    }
    context->Builder->CreateStore(allocCall, arrayPointerOffset);

    {
        auto rhsPtrOffset = context->Builder->CreateStructGEP(llvmRecordType, rhs, 2, "rhs.ptr.offset");
        auto loadResult =
                context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), rhsPtrOffset);
        auto bounds =
                context->Builder->CreateGEP(valueType, allocCall, llvm::ArrayRef<llvm::Value *>{lhsIndex}, "", false);

        std::vector<llvm::Value *> memcopyArgs;
        memcopyArgs.push_back(context->Builder->CreateBitCast(bounds, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateBitCast(loadResult, context->Builder->getPtrTy()));
        memcopyArgs.push_back(rhsIndex);
        memcopyArgs.push_back(context->Builder->getFalse());

        context->Builder->CreateCall(memcpyCall, memcopyArgs);
    }

    return stringAlloc;
}

llvm::Value *BinaryOperationNode::codegen(std::unique_ptr<Context> &context)
{

    llvm::Value *lhs = m_lhs->codegen(context);
    llvm::Value *rhs = m_rhs->codegen(context);
    if (!lhs || !rhs)
        return nullptr;

    ASTNode *parent = context->ProgramUnit.get();
    if (context->TopLevelFunction)
    {
        auto def = context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
        if (def)
        {
            parent = def.value().get();
        }
    }

    auto type = m_lhs->resolveType(context->ProgramUnit, parent);

    switch (type->baseType)
    {
        case VariableBaseType::Integer:
            return generateForInteger(lhs, rhs, context);
        case VariableBaseType::String:
            return generateForString(lhs, rhs, context);
        default:
            break;
    }

    return nullptr;
}
