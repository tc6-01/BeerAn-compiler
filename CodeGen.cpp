#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>
#include "CodeGen.h"
#include "ASTNodes.h"
#include "TypeSystem.h"
using legacy::PassManager;
#define ISTYPE(value, id) (value->getType()->getTypeID() == id)

static Type* TypeOf(const WangIdentifier & type, CodeGenContext& context){        // get llvm::type of variable base on its identifier
    return context.typeSystem.getVarType(type);
}

static Value* CastToBoolean(CodeGenContext& context, Value* condValue){

    if( ISTYPE(condValue, Type::IntegerTyID) ){
        condValue = context.builder.CreateIntCast(condValue, Type::getInt1Ty(context.llvmContext), true);
        return context.builder.CreateICmpNE(condValue, ConstantInt::get(Type::getInt1Ty(context.llvmContext), 0, true));
    }else if( ISTYPE(condValue, Type::DoubleTyID) ){
        return context.builder.CreateFCmpONE(condValue, ConstantFP::get(context.llvmContext, APFloat(0.0)));
    }else{
        return condValue;
    }
}

// 生成block的中间代码-----即程序入口
void CodeGenContext::generateCode(WangBlock& root) {
    cout << "##################################开始进入中间代码生成######################################" << endl;

    std::vector<Type*> sysArgs;
    FunctionType* mainFuncType = FunctionType::get(Type::getVoidTy(this->llvmContext), makeArrayRef(sysArgs), false);
    Function* mainFunc = Function::Create(mainFuncType, GlobalValue::ExternalLinkage, "BeerAn");
    BasicBlock* block = BasicBlock::Create(this->llvmContext, "程序");

    pushBlock(block);
    Value* retValue = root.codeGen(*this);
    popBlock();

    cout << "##################################已成功生成IR code#######################################" << endl;

    PassManager passManager;
    passManager.add(createPrintModulePass(outs()));
    passManager.run(*(this->theModule.get()));
    return;
}

llvm::Value* WangAssignment::codeGen(CodeGenContext &context) {
    cout << "生成赋值语句" << this->lhs->name << " = " << endl;
    Value* dst = context.getSymbolValue(this->lhs->name);
    auto dstType = context.getSymbolType(this->lhs->name);
    string dstTypeStr = dstType->name;
    if( !dst ){
        return LogErrorV("变量未定义");
    }
    Value* exp = exp = this->rhs->codeGen(context);
    context.builder.CreateStore(exp, dst);
    return dst;
}

llvm::Value* WangBinOp::codeGen(CodeGenContext &context) {
    cout << "生成二元运算操作" << endl;

    Value* L = this->lhs->codeGen(context);
    Value* R = this->rhs->codeGen(context);
    bool fp = false;

    if( (L->getType()->getTypeID() == Type::DoubleTyID) || (R->getType()->getTypeID() == Type::DoubleTyID) ){  // type upgrade
        fp = true;
        if( (R->getType()->getTypeID() != Type::DoubleTyID) ){
            R = context.builder.CreateUIToFP(R, Type::getDoubleTy(context.llvmContext), "ftmp");
        }
        if( (L->getType()->getTypeID() != Type::DoubleTyID) ){
            L = context.builder.CreateUIToFP(L, Type::getDoubleTy(context.llvmContext), "ftmp");
        }
    }

    if( !L || !R ){
        return nullptr;
    }
    cout << "fp = " << ( fp ? "true" : "false" ) << endl;

    switch (this->op){
        case TPLUS:
            return fp ? context.builder.CreateFAdd(L, R, "addftmp") : context.builder.CreateAdd(L, R, "addtmp");
        case TMINUS:
            return fp ? context.builder.CreateFSub(L, R, "subftmp") : context.builder.CreateSub(L, R, "subtmp");
        case TMUL:
            return fp ? context.builder.CreateFMul(L, R, "mulftmp") : context.builder.CreateMul(L, R, "multmp");
        case TDIV:
            return fp ? context.builder.CreateFDiv(L, R, "divftmp") : context.builder.CreateSDiv(L, R, "divtmp");
        case TAND:
            return fp ? LogErrorV("Double type has no AND operation") : context.builder.CreateAnd(L, R, "andtmp");
        case TOR:
            return fp ? LogErrorV("Double type has no OR operation") : context.builder.CreateOr(L, R, "ortmp");
        case TXOR:
            return fp ? LogErrorV("Double type has no XOR operation") : context.builder.CreateXor(L, R, "xortmp");
        case TSHIFTL:
            return fp ? LogErrorV("Double type has no LEFT SHIFT operation") : context.builder.CreateShl(L, R, "shltmp");
        case TSHIFTR:
            return fp ? LogErrorV("Double type has no RIGHT SHIFT operation") : context.builder.CreateAShr(L, R, "ashrtmp");

        case TCLT:
            return fp ? context.builder.CreateFCmpULT(L, R, "cmpftmp") : context.builder.CreateICmpULT(L, R, "cmptmp");
        case TCLE:
            return fp ? context.builder.CreateFCmpOLE(L, R, "cmpftmp") : context.builder.CreateICmpSLE(L, R, "cmptmp");
        case TCGE:
            return fp ? context.builder.CreateFCmpOGE(L, R, "cmpftmp") : context.builder.CreateICmpSGE(L, R, "cmptmp");
        case TCGT:
            return fp ? context.builder.CreateFCmpOGT(L, R, "cmpftmp") : context.builder.CreateICmpSGT(L, R, "cmptmp");
        case TCEQ:
            return fp ? context.builder.CreateFCmpOEQ(L, R, "cmpftmp") : context.builder.CreateICmpEQ(L, R, "cmptmp");
        case TCNE:
            return fp ? context.builder.CreateFCmpONE(L, R, "cmpftmp") : context.builder.CreateICmpNE(L, R, "cmptmp");
        default:
            return LogErrorV("Unknown binary operator");
    }
}

llvm::Value* WangBlock::codeGen(CodeGenContext &context) {
    cout << "Generating block" << endl;
    Value* last = nullptr;
    for(auto it=this->statements->begin(); it!=this->statements->end(); it++){
        last = (*it)->codeGen(context);
    }
    return last;
}

llvm::Value* WangInteger::codeGen(CodeGenContext &context) {
    cout << "Generating Integer: " << this->value << endl;
    return ConstantInt::get(Type::getInt32Ty(context.llvmContext), this->value, true);
//    return ConstantInt::get(context.llvmContext, APInt(INTBITS, this->value, true));
}

llvm::Value* WangDouble::codeGen(CodeGenContext &context) {
    cout << "Generating Double: " << this->value << endl;
    return ConstantFP::get(Type::getDoubleTy(context.llvmContext), this->value);
//    return ConstantFP::get(context.llvmContext, APFloat(this->value));
}

llvm::Value* WangIdentifier::codeGen(CodeGenContext &context) {
    cout << "生成变量 " << this->name << endl;
    Value* value = context.getSymbolValue(this->name);
    if( !value ){
        return LogErrorV("Unknown variable name " + this->name);
    }
    return context.builder.CreateLoad(value, false, "");

}

llvm::Value* WangExpressionStatement::codeGen(CodeGenContext &context) {
    return this->expression->codeGen(context);
}

llvm::Value* WangFuncDel::codeGen(CodeGenContext &context) {
    cout << "Generating function declaration of " << this->id->name << endl;
    std::vector<Type*> argTypes;

    for(auto &arg: *this->arguments){
        argTypes.push_back(TypeOf(*arg->type, context));
    }
    Type* retType = nullptr;
    retType = TypeOf(*this->type, context);

    FunctionType* functionType = FunctionType::get(retType, argTypes, false);
    Function* function = Function::Create(functionType, GlobalValue::ExternalLinkage, this->id->name.c_str(), context.theModule.get());

    if( !this->isExternal ){
        BasicBlock* basicBlock = BasicBlock::Create(context.llvmContext, "entry", function, nullptr);

        context.builder.SetInsertPoint(basicBlock);
        context.pushBlock(basicBlock);

        // declare function params
        auto origin_arg = this->arguments->begin();

        for(auto &ir_arg_it: function->args()){
            ir_arg_it.setName((*origin_arg)->id->name);
            Value* argAlloc;
            argAlloc = (*origin_arg)->codeGen(context);

            context.builder.CreateStore(&ir_arg_it, argAlloc, false);
            context.setSymbolValue((*origin_arg)->id->name, argAlloc);
            context.setSymbolType((*origin_arg)->id->name, (*origin_arg)->type);
            context.setFuncArg((*origin_arg)->id->name, true);
            origin_arg++;
        }

        this->block->codeGen(context);
        if( context.getCurrentReturnValue() ){
            context.builder.CreateRet(context.getCurrentReturnValue());
        } else{
            return LogErrorV("Function block return value not founded");
        }
        context.popBlock();

    }


    return function;
}

llvm::Value* WangCallFunc::codeGen(CodeGenContext &context) {
    cout << "Generating method call of " << this->id->name << endl;
    Function * calleeF = context.theModule->getFunction(this->id->name);
    if( !calleeF ){
        LogErrorV("Function name not found");
    }
    if( calleeF->arg_size() != this->arguments->size() ){
        LogErrorV("Function arguments size not match, calleeF=" + std::to_string(calleeF->size()) + ", this->arguments=" + std::to_string(this->arguments->size()) );
    }
    std::vector<Value*> argsv;
    for(auto it=this->arguments->begin(); it!=this->arguments->end(); it++){
        argsv.push_back((*it)->codeGen(context));
        if( !argsv.back() ){        // if any argument codegen fail
            return nullptr;
        }
    }
    return context.builder.CreateCall(calleeF, argsv, "calltmp");
}

llvm::Value* WangVarDel::codeGen(CodeGenContext &context) {
    cout << "Generating variable declaration of " << this->type->name << " " << this->id->name << endl;
    Type* type = TypeOf(*this->type, context);
    Value* initial = nullptr;

    Value* inst = nullptr;
    inst = context.builder.CreateAlloca(type);


    context.setSymbolType(this->id->name, this->type);
    context.setSymbolValue(this->id->name, inst);

    context.PrintSymTable();

    if( this->assignmentExpr != nullptr ){
        WangAssignment assignment(this->id, this->assignmentExpr);
        assignment.codeGen(context);
    }
    return inst;
}

llvm::Value* WangReturn::codeGen(CodeGenContext &context) {
    cout << "Generating return statement" << endl;
    Value* returnValue = this->expression->codeGen(context);
    context.setCurrentReturnValue(returnValue);
    return returnValue;
}

llvm::Value* WangIf::codeGen(CodeGenContext &context) {
    cout << "Generating if statement" << endl;
    Value* condValue = this->condition->codeGen(context);
    if( !condValue )
        return nullptr;

    condValue = CastToBoolean(context, condValue);

    Function* theFunction = context.builder.GetInsertBlock()->getParent();      // the function where if statement is in

    BasicBlock *thenBB = BasicBlock::Create(context.llvmContext, "then", theFunction);
    BasicBlock *falseBB = BasicBlock::Create(context.llvmContext, "else");
    BasicBlock *mergeBB = BasicBlock::Create(context.llvmContext, "ifcont");

    if( this->falseBlock ){
        context.builder.CreateCondBr(condValue, thenBB, falseBB);
    } else{
        context.builder.CreateCondBr(condValue, thenBB, mergeBB);
    }

    context.builder.SetInsertPoint(thenBB);

    context.pushBlock(thenBB);

    this->trueBlock->codeGen(context);

    context.popBlock();

    thenBB = context.builder.GetInsertBlock();

    if( thenBB->getTerminator() == nullptr ){       //
        context.builder.CreateBr(mergeBB);
    }

    if( this->falseBlock ){
        theFunction->getBasicBlockList().push_back(falseBB);    //
        context.builder.SetInsertPoint(falseBB);            //

        context.pushBlock(thenBB);

        this->falseBlock->codeGen(context);

        context.popBlock();

        context.builder.CreateBr(mergeBB);
    }

    theFunction->getBasicBlockList().push_back(mergeBB);        //
    context.builder.SetInsertPoint(mergeBB);        //

    return nullptr;
}

llvm::Value* WangFOR::codeGen(CodeGenContext &context) {

    Function* theFunction = context.builder.GetInsertBlock()->getParent();

    BasicBlock *block = BasicBlock::Create(context.llvmContext, "for循环", theFunction);
    BasicBlock *after = BasicBlock::Create(context.llvmContext, "for后程序");

    // 初始化
    if( this->initial )
        this->initial->codeGen(context);

    // 条件判断
    Value* condValue = this->condition->codeGen(context);
    if( !condValue )
        return nullptr;

    condValue = CastToBoolean(context, condValue);

    context.builder.CreateCondBr(condValue, block, after);

    context.builder.SetInsertPoint(block);

    context.pushBlock(block);

    this->block->codeGen(context);

    context.popBlock();

    // 变化操作
    if( this->increment ){ 
        this->increment->codeGen(context);
    }

    condValue = this->condition->codeGen(context);
    condValue = CastToBoolean(context, condValue);
    context.builder.CreateCondBr(condValue, block, after);

    // 插入for循环后续
    theFunction->getBasicBlockList().push_back(after);
    context.builder.SetInsertPoint(after);

    return nullptr;
}

std::unique_ptr<WangExpression> LogError(const char *str) {
    fprintf(stderr, "LogError: %s\n", str);
    return nullptr;
}

Value *LogErrorV(string str){
    return LogErrorV(str.c_str());
}

Value *LogErrorV(const char *str) {
    LogError(str);
    return nullptr;
}
