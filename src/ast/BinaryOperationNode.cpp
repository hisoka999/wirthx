#include "BinaryOperationNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>

#include "UnitNode.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"
#include "types/StringType.h"

BinaryOperationNode::BinaryOperationNode(const Token &operatorToken, const Operator op,
                                         const std::shared_ptr<ASTNode> &lhs, const std::shared_ptr<ASTNode> &rhs) :
    ASTNode(operatorToken), m_operatorToken(operatorToken), m_lhs(lhs), m_rhs(rhs), m_operator(op)
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
    const size_t maxBitWith = std::max(lhs->getType()->getIntegerBitWidth(), rhs->getType()->getIntegerBitWidth());
    const auto targetType = llvm::IntegerType::get(*context->TheContext, maxBitWith);
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
        case Operator::MOD:
            return context->Builder->CreateSRem(lhs, rhs, "srem");
        case Operator::DIV:
            return context->Builder->CreateSDiv(lhs, rhs, "sdiv");
    }
    return nullptr;
}

llvm::Value *BinaryOperationNode::generateForStringPlusInteger(llvm::Value *lhs, llvm::Value *rhs,
                                                               std::unique_ptr<Context> &context)
{
    const auto varType = StringType::getString();
    const auto valueType = VariableType::getInteger(8)->generateLlvmType(context);
    const auto llvmRecordType = varType->generateLlvmType(context);
    const auto indexType = VariableType::getInteger(64)->generateLlvmType(context);
    const auto stringAlloc = context->Builder->CreateAlloca(llvmRecordType, nullptr, "combined_string");


    const auto arrayRefCountOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 0, "combined_string.refCount.offset");
    const auto arraySizeOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 1, "combined_string.size.offset");


    const auto arrayPointerOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 2, "combined_string.ptr.offset");

    const auto lhsIndexPtr = context->Builder->CreateStructGEP(llvmRecordType, lhs, 1, "lhs.size.offset");


    // lhs size
    const auto lhsIndex = context->Builder->CreateLoad(indexType, lhsIndexPtr, "lhs.size");

    llvm::Value *rhsSize = nullptr;
    if (rhs->getType()->getIntegerBitWidth() == 8)
    {
        rhsSize = context->Builder->getInt64(1);
    }

    const auto newSize = context->Builder->CreateAdd(lhsIndex, rhsSize, "new_size");


    // change array size
    context->Builder->CreateStore(context->Builder->getInt64(1), arrayRefCountOffset);
    context->Builder->CreateStore(newSize, arraySizeOffset);
    const auto allocCall =
            context->Builder->CreateCall(context->TheModule->getFunction("malloc"),
                                         context->Builder->CreateAdd(newSize, context->Builder->getInt64(1)));

    {
        const auto memcpyCall = llvm::Intrinsic::getDeclaration(
                context->TheModule.get(), llvm::Intrinsic::memcpy,
                {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
        const auto boundsLhs = context->Builder->CreateGEP(
                valueType, allocCall, llvm::ArrayRef<llvm::Value *>{context->Builder->getInt64(0)}, "", false);
        const auto lhsPtrOffset = context->Builder->CreateStructGEP(llvmRecordType, lhs, 2, "lhs.ptr.offset");
        const auto loadResult =
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

        const auto bounds =
                context->Builder->CreateGEP(valueType, allocCall, llvm::ArrayRef<llvm::Value *>{lhsIndex}, "", false);

        context->Builder->CreateStore(rhs, bounds);
    }
    {

        const auto bounds = context->Builder->CreateGEP(
                valueType, allocCall,
                llvm::ArrayRef<llvm::Value *>{context->Builder->CreateAdd(lhsIndex, context->Builder->getInt64(1))}, "",
                false);

        context->Builder->CreateStore(context->Builder->getInt8(0), bounds);
    }
    return stringAlloc;
}

llvm::Value *BinaryOperationNode::generateForString(llvm::Value *lhs, llvm::Value *rhs,
                                                    std::unique_ptr<Context> &context)
{

    const auto varType = StringType::getString();
    const auto valueType = VariableType::getInteger(8)->generateLlvmType(context);
    const auto llvmRecordType = varType->generateLlvmType(context);
    const auto indexType = VariableType::getInteger(64)->generateLlvmType(context);
    const auto stringAlloc = context->Builder->CreateAlloca(llvmRecordType, nullptr, "combined_string");


    const auto arrayRefCountOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 0, "combined_string.refCount.offset");
    const auto arraySizeOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 1, "combined_string.size.offset");


    const auto arrayPointerOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 2, "combined_string.ptr.offset");

    const auto lhsIndexPtr = context->Builder->CreateStructGEP(llvmRecordType, lhs, 1, "lhs.size.offset");

    const auto rhsIndexPtr = context->Builder->CreateStructGEP(llvmRecordType, rhs, 1, "rhs.size.offset");
    // lhs size
    const auto lhsIndex = context->Builder->CreateLoad(indexType, lhsIndexPtr, "lhs.size");
    const auto rhsIndex = context->Builder->CreateLoad(indexType, rhsIndexPtr, "rhs.size");

    const auto newSize = context->Builder->CreateAdd(
            lhsIndex, context->Builder->CreateAdd(rhsIndex, context->Builder->getInt64(1)), "new_size");
    ;


    // change array size
    context->Builder->CreateStore(context->Builder->getInt64(1), arrayRefCountOffset);
    context->Builder->CreateStore(newSize, arraySizeOffset);
    const auto allocCall =
            context->Builder->CreateCall(context->TheModule->getFunction("malloc"),
                                         context->Builder->CreateAdd(newSize, context->Builder->getInt64(1)));

    const auto memcpyCall = llvm::Intrinsic::getDeclaration(
            context->TheModule.get(), llvm::Intrinsic::memcpy,
            {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
    {
        const auto boundsLhs = context->Builder->CreateGEP(
                valueType, allocCall, llvm::ArrayRef<llvm::Value *>{context->Builder->getInt64(0)}, "", false);
        const auto lhsPtrOffset = context->Builder->CreateStructGEP(llvmRecordType, lhs, 2, "lhs.ptr.offset");
        const auto loadResult =
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
        const auto rhsPtrOffset = context->Builder->CreateStructGEP(llvmRecordType, rhs, 2, "rhs.ptr.offset");
        const auto loadResult =
                context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), rhsPtrOffset);
        const auto bounds =
                context->Builder->CreateGEP(valueType, allocCall, llvm::ArrayRef<llvm::Value *>{lhsIndex}, "", false);

        std::vector<llvm::Value *> memcopyArgs;
        memcopyArgs.push_back(context->Builder->CreateBitCast(bounds, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateBitCast(loadResult, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateAdd(rhsIndex, context->Builder->getInt64(1), "", false));
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
        if (const auto def = context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str()))
        {
            parent = def.value().get();
        }
    }

    if (lhs->getType()->isIntegerTy())
    {
        return generateForInteger(lhs, rhs, context);
    }

    const auto lhs_type = m_lhs->resolveType(context->ProgramUnit, parent);
    const auto rhs_type = m_rhs->resolveType(context->ProgramUnit, parent);


    switch (lhs_type->baseType)
    {
        case VariableBaseType::Integer:
            return generateForInteger(lhs, rhs, context);
        case VariableBaseType::String:
            switch (rhs_type->baseType)
            {
                case VariableBaseType::String:
                    return generateForString(lhs, rhs, context);
                case VariableBaseType::Integer:
                    return generateForStringPlusInteger(lhs, rhs, context);
                default:
                    assert(false && "unknown variable type for binary opteration");
                    break;
            }

        default:
            assert(false && "unknown variable type for binary opteration");
            break;
    }

    return nullptr;
}

std::shared_ptr<VariableType> BinaryOperationNode::resolveType(const std::unique_ptr<UnitNode> &unit,
                                                               ASTNode *parentNode)
{
    if (auto type = m_lhs->resolveType(unit, parentNode))
    {
        return type;
    }
    return std::make_shared<VariableType>();
}
void BinaryOperationNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    if (const auto lhsType = m_lhs->resolveType(unit, parentNode); auto rhsType = m_rhs->resolveType(unit, parentNode))
    {
        if (*lhsType != *rhsType)
        {
            if (not(lhsType->baseType == VariableBaseType::String && rhsType->baseType == VariableBaseType::Integer))
            {
                throw CompilerException(ParserError{
                        .token = m_operatorToken,
                        .message = "the binary operation of \"" + lhsType->typeName + "\" and \"" + rhsType->typeName +
                                   "\" is not possible because the types are not the same"});
            }
        }
    }
}
