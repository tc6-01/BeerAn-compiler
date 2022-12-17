#ifndef __CODEGEN_H__
#define __CODEGEN_H__


#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <stack>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "ASTNodes.h"
#include "BeerAnSytnax.hpp"
#include "TypeSystem.h"
using namespace llvm;
using std::unique_ptr;
using std::string;


class CodeGenBlock{
public:
    BasicBlock * block;
    Value * returnValue;
    // 变量表
    std::map<string, Value*> locals;
    // 与变量表对应的类型表
    std::map<string, shared_ptr<WangIdentifier>> types;   
    // 函数表 
    std::map<string, bool> isFuncArg;
    std::map<string, std::vector<uint64_t>> arraySizes;
};

class CodeGenContext{
private:
    // 基本块儿用栈存储
    std::vector<CodeGenBlock*> blockStack;
public:
    LLVMContext llvmContext;
    IRBuilder<> builder;
    unique_ptr<Module> theModule;
    TypeSystem typeSystem;

    CodeGenContext(): builder(llvmContext), typeSystem(llvmContext){
        theModule = unique_ptr<Module>(new Module("BeerAn", this->llvmContext));
    }
    // 查找符号表中变量的值
    Value* getSymbolValue(string name) const{
        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->locals.find(name) != (*it)->locals.end() ){
                return (*it)->locals[name];
            }
        }
        return nullptr;
    }
    // 查找符号表中变量的类型
    shared_ptr<WangIdentifier> getSymbolType(string name) const{
        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->types.find(name) != (*it)->types.end() ){
                return (*it)->types[name];
            }
        }
        return nullptr;
    }
    // 判断调用函数是否声明
    bool isFuncArg(string name) const{

        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->isFuncArg.find(name) != (*it)->isFuncArg.end() ){
                return (*it)->isFuncArg[name];
            }
        }
        return false;
    }
    // 添加变量进入符号表
    void setSymbolValue(string name, Value* value){
        blockStack.back()->locals[name] = value;
    }
    // 添加符号表中变量类型
    void setSymbolType(string name, shared_ptr<WangIdentifier> value){
        blockStack.back()->types[name] = value;
    }
    // 声明函数，将函数加入函数表中
    void setFuncArg(string name, bool value){
        cout << "添加 " << name << " 作为函数参数" << endl;
        blockStack.back()->isFuncArg[name] = value;
    }
    // 
    BasicBlock* currentBlock() const{
        return blockStack.back()->block;
    }
    // 基本块儿进栈
    void pushBlock(BasicBlock * block){
        CodeGenBlock * codeGenBlock = new CodeGenBlock();
        codeGenBlock->block = block;
        codeGenBlock->returnValue = nullptr;
        blockStack.push_back(codeGenBlock);
    }
    // 基本块儿出栈
    void popBlock(){
        CodeGenBlock * codeGenBlock = blockStack.back();
        blockStack.pop_back();
        delete codeGenBlock;
    }
    // 添加当前基本块儿的返回值
    void setCurrentReturnValue(Value* value){
        blockStack.back()->returnValue = value;
    }
    // 得到当前基本块儿的返回值
    Value* getCurrentReturnValue(){
        return blockStack.back()->returnValue;
    }
    // 打印符号表
    void PrintSymTable() const{
        cout << "#############################符号表#################################" << endl;
        string prefix = "";
        for(auto it=blockStack.begin(); it!=blockStack.end(); it++){
            for(auto it2=(*it)->locals.begin(); it2!=(*it)->locals.end(); it2++){
                cout << prefix << it2->first << " = " << it2->second << ": " << this->getSymbolType(it2->first) << endl;
            }
            prefix += "\t";
        }
        cout << "#####################################################################" << endl;
    }

    void generateCode(WangBlock& );
};

Value* LogErrorV(const char* str);
Value* LogErrorV(string str);

#endif