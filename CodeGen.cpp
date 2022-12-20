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
    // 返回变量类型的llvm值
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
    // 根据输入的条件转换为布尔值，包括整数，浮点数，条件判断
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
    // reference: （llvm官方入手文档）https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html
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
    // 确定是否为浮点数运算
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
            return fp ? context.builder.CreateFAdd(L, R, "addF") : context.builder.CreateAdd(L, R, "add");
        case TMINUS:
            cout<<"操作: "<<"-"<<endl;
            return fp ? context.builder.CreateFSub(L, R, "subF") : context.builder.CreateSub(L, R, "sub");
        case TMUL:
            cout<<"操作: "<<"*"<<endl;
            return fp ? context.builder.CreateFMul(L, R, "mulF") : context.builder.CreateMul(L, R, "mul");
        case TDIV:
            cout<<"操作: "<<"/"<<endl;
            return fp ? context.builder.CreateFDiv(L, R, "divF") : context.builder.CreateSDiv(L, R, "div");
        case TAND:
            cout<<"操作: "<<"&"<<endl;
            return fp ? LogErrorV("and error") : context.builder.CreateAnd(L, R, "and");
        case TOR:
            cout<<"操作: "<<"|"<<endl;
            return fp ? LogErrorV("or error") : context.builder.CreateOr(L, R, "or");
        case TXOR:
            cout<<"操作: "<<"^"<<endl;
            return fp ? LogErrorV("xor error") : context.builder.CreateXor(L, R, "xor");
        case TSHIFTL:
            cout<<"操作: "<<">>"<<endl;
            return fp ? LogErrorV("left shift error") : context.builder.CreateShl(L, R, "left shift");
        case TSHIFTR:
            cout<<"操作: "<<"<<"<<endl;
            return fp ? LogErrorV("right shift error") : context.builder.CreateAShr(L, R, "right shift");
        case TCLT:
            cout<<"操作: "<<"<"<<endl;
            return fp ? context.builder.CreateFCmpULT(L, R, "Fless op") : context.builder.CreateICmpULT(L, R, "less");
        case TCLE:
            cout<<"操作: "<<"<="<<endl;
            return fp ? context.builder.CreateFCmpOLE(L, R, "FlessE op") : context.builder.CreateICmpSLE(L, R, "less equal");
        case TCGE:
            cout<<"操作: "<<">="<<endl;
            return fp ? context.builder.CreateFCmpOGE(L, R, "FbiggerE op") : context.builder.CreateICmpSGE(L, R, "bigger equal");
        case TCGT:
            cout<<"操作: "<<">"<<endl;
            return fp ? context.builder.CreateFCmpOGT(L, R, "Fbigger op") : context.builder.CreateICmpSGT(L, R, "bigger");
        case TCEQ:
            cout<<"操作: "<<"=="<<endl;
            return fp ? context.builder.CreateFCmpOEQ(L, R, "FEE op") : context.builder.CreateICmpEQ(L, R, "equal equal");
        case TCNE:
            cout<<"操作: "<<"!="<<endl;
            return fp ? context.builder.CreateFCmpONE(L, R, "Fne op") : context.builder.CreateICmpNE(L, R, "not equal");
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

llvm::Value* WangIf::codeGen(CodeGenContext &context) {
    cout << "生成if条件语句" << endl;
    Value* condValue = this->condition->codeGen(context);
    if( !condValue )
        return nullptr;

    condValue = CastToBoolean(context, condValue);
    // 获取当前if语句所在的基本块儿
    Function* CurFunc = context.builder.GetInsertBlock()->getParent();  

    BasicBlock *thenBB = BasicBlock::Create(context.llvmContext, "then", CurFunc);
    BasicBlock *falseBB = BasicBlock::Create(context.llvmContext, "else");
    BasicBlock *mergeBB = BasicBlock::Create(context.llvmContext, "afetr if");

    // 条件判断
    if( this->falseBlock ){
        context.builder.CreateCondBr(condValue, thenBB, falseBB);
    } else{
        context.builder.CreateCondBr(condValue, thenBB, mergeBB);
    }
    // if执行内容
    context.builder.SetInsertPoint(thenBB);
    context.pushBlock(thenBB);
    this->trueBlock->codeGen(context);
    context.popBlock();
    thenBB = context.builder.GetInsertBlock();

    if( thenBB->getTerminator() == nullptr ){      
        context.builder.CreateBr(mergeBB);
    }
    // else
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
    // 类似于if，for同样需要获取当前所在的基本块儿
    Function* CurFunc = context.builder.GetInsertBlock()->getParent();

    BasicBlock *block = BasicBlock::Create(context.llvmContext, "for loop", CurFunc);
    BasicBlock *after = BasicBlock::Create(context.llvmContext, "after for");

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

    // 插入for循环后续
    CurFunc->getBasicBlockList().push_back(after);
    context.builder.SetInsertPoint(after);

    return nullptr;
}

llvm::Value* WangFuncDel::codeGen(CodeGenContext &context) {
    cout << "生成函数声明" << this->id->name << endl;
    std::vector<Type*> argTypes;

    for(auto &arg: *this->arguments){
        argTypes.push_back(TypeOf(*arg->type, context));
    }
    Type* retType = nullptr;
    retType = TypeOf(*this->type, context);
    // 获取函数返回类型与函数对象----自动添加进入函数表
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
        cout << "添加 " << (*origin_arg)->id->name << " 作为函数参数" << endl;
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
    // 获取调用函数对象
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
        if( !argsv.back() ){       
            return nullptr;
        }
    }
    return context.builder.CreateCall(calleeF, argsv, "调用");
}

llvm::Value* WangReturn::codeGen(CodeGenContext &context) {
    cout << "生成返回值" << endl;
    Value* returnValue = this->expression->codeGen(context);
    context.setCurrentRetValue(returnValue);
    return returnValue;
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
