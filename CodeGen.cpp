#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>
#include "CodeGen.h"
#include "ASTNodes.h"
using legacy::PassManager;
#define ISTYPE(value, id) (value->getType()->getTypeID() == id)

Type *CodeGenContext::getVarType(const WangIdentifier& type) {

    string typeStr = type.name;
    if( typeStr.compare("int") == 0 ){
        return this->intTy; 
    }
    if( typeStr.compare("double") == 0 ){
        return this->doubleTy;
    }
    return nullptr;
}
static Type* TypeOf(const WangIdentifier & type, CodeGenContext& context){   
    return context.getVarType(type);
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
// reference:
    std::vector<Type*> sysArgs;
    FunctionType* fp = FunctionType::get(Type::getVoidTy(this->llvmContext), makeArrayRef(sysArgs), false);
    Function* func = Function::Create(fp, GlobalValue::ExternalLinkage, "BeerAn");
    BasicBlock* block = BasicBlock::Create(this->llvmContext, "entry");

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

    if( (L->getType()->getTypeID() == Type::DoubleTyID) || (R->getType()->getTypeID() == Type::DoubleTyID) ){ 
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
    switch (this->op){
        case TPLUS:
            cout<<"操作: "<<"+"<<endl;
            return fp ? context.builder.CreateFAdd(L, R, "加法错误") : context.builder.CreateAdd(L, R, "加法");
        case TMINUS:
            cout<<"操作: "<<"-"<<endl;
            return fp ? context.builder.CreateFSub(L, R, "减法错误") : context.builder.CreateSub(L, R, "减法");
        case TMUL:
            cout<<"操作: "<<"*"<<endl;
            return fp ? context.builder.CreateFMul(L, R, "乘法错误") : context.builder.CreateMul(L, R, "乘法");
        case TDIV:
            cout<<"操作: "<<"/"<<endl;
            return fp ? context.builder.CreateFDiv(L, R, "除法错误") : context.builder.CreateSDiv(L, R, "除法");
        case TAND:
            cout<<"操作: "<<"&"<<endl;
            return fp ? LogErrorV("操作有误") : context.builder.CreateAnd(L, R, "与操作");
        case TOR:
            cout<<"操作: "<<"|"<<endl;
            return fp ? LogErrorV("操作有误") : context.builder.CreateOr(L, R, "或操作");
        case TXOR:
            cout<<"操作: "<<"^"<<endl;
            return fp ? LogErrorV("操作有误") : context.builder.CreateXor(L, R, "异或操作");
        case TSHIFTL:
            cout<<"操作: "<<">>"<<endl;
            return fp ? LogErrorV("操作有误") : context.builder.CreateShl(L, R, "左移");
        case TSHIFTR:
            cout<<"操作: "<<"<<"<<endl;
            return fp ? LogErrorV("操作有误") : context.builder.CreateAShr(L, R, "右移");
        case TCLT:
            cout<<"操作: "<<"<"<<endl;
            return fp ? context.builder.CreateFCmpULT(L, R, "比较错误") : context.builder.CreateICmpULT(L, R, "比较");
        case TCLE:
            cout<<"操作: "<<"<="<<endl;
            return fp ? context.builder.CreateFCmpOLE(L, R, "比较错误") : context.builder.CreateICmpSLE(L, R, "比较");
        case TCGE:
            cout<<"操作: "<<">="<<endl;
            return fp ? context.builder.CreateFCmpOGE(L, R, "比较错误") : context.builder.CreateICmpSGE(L, R, "比较");
        case TCGT:
            cout<<"操作: "<<">"<<endl;
            return fp ? context.builder.CreateFCmpOGT(L, R, "比较错误") : context.builder.CreateICmpSGT(L, R, "比较");
        case TCEQ:
            cout<<"操作: "<<"=="<<endl;
            return fp ? context.builder.CreateFCmpOEQ(L, R, "比较错误") : context.builder.CreateICmpEQ(L, R, "比较");
        case TCNE:
            cout<<"操作: "<<"!="<<endl;
            return fp ? context.builder.CreateFCmpONE(L, R, "比较错误") : context.builder.CreateICmpNE(L, R, "比较");
        default:
            return LogErrorV("Unknown binary operator");
    }
}

llvm::Value* WangBlock::codeGen(CodeGenContext &context) {
    cout << "生成基本块儿" << endl;
    Value* last = nullptr;
    for(auto it=this->statements->begin(); it!=this->statements->end(); it++){
        last = (*it)->codeGen(context);
    }
    return last;
}

llvm::Value* WangInteger::codeGen(CodeGenContext &context) {
    cout << "生成整型数: " << this->value << endl;
    return ConstantInt::get(Type::getInt32Ty(context.llvmContext), this->value, true);
}

llvm::Value* WangDouble::codeGen(CodeGenContext &context) {
    cout << "生成浮点数: " << this->value << endl;
    return ConstantFP::get(Type::getDoubleTy(context.llvmContext), this->value);
}

llvm::Value* WangIdentifier::codeGen(CodeGenContext &context) {
    cout << "生成变量 " << this->name << endl;
    Value* value = context.getSymbolValue(this->name);
    if( !value ){
        return LogErrorV("未知变量" + this->name);
    }
    return context.builder.CreateLoad(value, false, "");
}

llvm::Value* WangExpressionStatement::codeGen(CodeGenContext &context) {
    return this->expression->codeGen(context);
}

llvm::Value* WangFuncDel::codeGen(CodeGenContext &context) {
    cout << "生成函数声明" << this->id->name << endl;
    std::vector<Type*> argTypes;

    for(auto &arg: *this->arguments){
        argTypes.push_back(TypeOf(*arg->type, context));
    }
    Type* retType = nullptr;
    retType = TypeOf(*this->type, context);

    FunctionType* functionType = FunctionType::get(retType, argTypes, false);
    Function* function = Function::Create(functionType, GlobalValue::ExternalLinkage, this->id->name.c_str(), context.theModule.get());

    BasicBlock* basicBlock = BasicBlock::Create(context.llvmContext, "entry", function, nullptr);

    context.builder.SetInsertPoint(basicBlock);
    context.pushBlock(basicBlock);
    auto origin_arg = this->arguments->begin();

    for(auto &ir_arg_it: function->args()){
        ir_arg_it.setName((*origin_arg)->id->name);
        Value* argAlloc;
        argAlloc = (*origin_arg)->codeGen(context);

        context.builder.CreateStore(&ir_arg_it, argAlloc, false);
        context.setSymbolValue((*origin_arg)->id->name, argAlloc);
        context.setSymbolType((*origin_arg)->id->name, (*origin_arg)->type->name);
        context.setFuncArg((*origin_arg)->id->name, true);
        origin_arg++;
    }

    this->block->codeGen(context);
    if( context.getCurrentRetValue() ){
        context.builder.CreateRet(context.getCurrentRetValue());
    } else{
        return LogErrorV("函数缺乏返回值");
    }
    context.popBlock();
    return function;
}

llvm::Value* WangCallFunc::codeGen(CodeGenContext &context) {
    cout << "生成函数调用 " << this->id->name << endl;
    Function * calleeF = context.theModule->getFunction(this->id->name);
    if( !calleeF ){
        LogErrorV("函数未声明");
    }
    if( calleeF->arg_size() != this->arguments->size() ){
        LogErrorV("参数与已声明不符, expected " + std::to_string(calleeF->arg_size()) + ", but the real is " + std::to_string(this->arguments->size()) );
    }
    std::vector<Value*> argsv;
    for(auto it=this->arguments->begin(); it!=this->arguments->end(); it++){
        argsv.push_back((*it)->codeGen(context));
        if( !argsv.back() ){        // if any argument codegen fail
            return nullptr;
        }
    }
    return context.builder.CreateCall(calleeF, argsv, "调用");
}

llvm::Value* WangVarDel::codeGen(CodeGenContext &context) {
    cout << "生成变量声明 " << this->type->name << " " << this->id->name << endl;
    Type* typeID = TypeOf(*this->type, context);
    Value* initial = nullptr;

    Value* inst = nullptr;
    inst = context.builder.CreateAlloca(typeID);
    // 在变量声明的时候添加进入符号表
    context.setSymbolType(this->id->name, this->type->name);
    context.setSymbolValue(this->id->name, inst);

    context.PrintSymTable();

    if( this->assignmentExpr != nullptr ){
        WangAssignment assignment(this->id, this->assignmentExpr);
        assignment.codeGen(context);
    }
    return inst;
}

llvm::Value* WangReturn::codeGen(CodeGenContext &context) {
    cout << "生成返回值" << endl;
    Value* returnValue = this->expression->codeGen(context);
    context.setCurrentRetValue(returnValue);
    return returnValue;
}

llvm::Value* WangIf::codeGen(CodeGenContext &context) {
    cout << "生成if条件语句" << endl;
    Value* condValue = this->condition->codeGen(context);
    if( !condValue )
        return nullptr;

    condValue = CastToBoolean(context, condValue);

    Function* CurFunc = context.builder.GetInsertBlock()->getParent();      // the function where if statement is in

    BasicBlock *thenBB = BasicBlock::Create(context.llvmContext, "then", CurFunc);
    BasicBlock *falseBB = BasicBlock::Create(context.llvmContext, "else");
    BasicBlock *mergeBB = BasicBlock::Create(context.llvmContext, "if后续");

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

    if( thenBB->getTerminator() == nullptr ){      
        context.builder.CreateBr(mergeBB);
    }

    if( this->falseBlock ){
        CurFunc->getBasicBlockList().push_back(falseBB);    
        context.builder.SetInsertPoint(falseBB);           

        context.pushBlock(thenBB);

        this->falseBlock->codeGen(context);

        context.popBlock();

        context.builder.CreateBr(mergeBB);
    }

    CurFunc->getBasicBlockList().push_back(mergeBB);   
    context.builder.SetInsertPoint(mergeBB);     

    return nullptr;
}

llvm::Value* WangFOR::codeGen(CodeGenContext &context) {
    cout << "生成for循环语句" << endl;
    Function* CurFunc = context.builder.GetInsertBlock()->getParent();

    BasicBlock *block = BasicBlock::Create(context.llvmContext, "for循环", CurFunc);
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
    CurFunc->getBasicBlockList().push_back(after);
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
