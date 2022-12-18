// 中间代码生成
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <stack>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "ASTNodes.h"
#include "BeerAnSytnax.hpp"
using namespace llvm;
using std::unique_ptr;
using std::string;

class CodeGenBlock{
public:
    // 基本块儿
    BasicBlock * block;
    Value * RetValue;
    // 变量表
    std::map<string, Value*> SymTable;
    // 与变量表对应的类型表
    std::map<string, string> types;   
    // 函数表 
    std::map<string, bool> FuncTable;
};

class CodeGenContext{
private:
    // 基本块儿用栈存储
    std::vector<CodeGenBlock*> blockStack;
public:
    LLVMContext llvmContext;
    IRBuilder<> builder;
    unique_ptr<Module> theModule;
    // 放入llvm Type值
    Type* intTy = Type::getInt32Ty(llvmContext);
    Type* doubleTy = Type::getDoubleTy(llvmContext);
    // 获取变量类型的llvm 对应值
    Type* getVarType(const WangIdentifier& type);

    CodeGenContext(): builder(llvmContext){
        theModule = unique_ptr<Module>(new Module("BeerAn", this->llvmContext));
    }
    // 查找符号表中变量的值
    Value* getSymbolValue(string name) const{
        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->SymTable.find(name) != (*it)->SymTable.end() ){
                return (*it)->SymTable[name];
            }
        }
        return nullptr;
    }
    // 查找符号表中变量的类型
    string getSymbolType(string name) const{
        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->types.find(name) != (*it)->types.end() ){
                return (*it)->types[name];
            }
        }
        return nullptr;
    }
    // 判断调用函数是否声明
    bool FuncTable(string name) const{
        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->FuncTable.find(name) != (*it)->FuncTable.end() ){
                return (*it)->FuncTable[name];
            }
        }
        return false;
    }
    // 添加变量进入符号表
    void setSymbolValue(string name, Value* value){
        blockStack.back()->SymTable[name] = value;
    }
    // 添加符号表中变量类型
    void setSymbolType(string name, string value){
        blockStack.back()->types[name] = value;
    }
    // 声明函数，将函数加入函数表中
    void setFuncArg(string name, bool value){
        cout << "添加 " << name << " 作为函数参数" << endl;
        blockStack.back()->FuncTable[name] = value;
    }
    // 基本块儿进栈
    void pushBlock(BasicBlock * block){
        CodeGenBlock * codeGenBlock = new CodeGenBlock();
        codeGenBlock->block = block;
        codeGenBlock->RetValue = nullptr;
        blockStack.push_back(codeGenBlock);
    }
    // 基本块儿出栈
    void popBlock(){
        CodeGenBlock * codeGenBlock = blockStack.back();
        blockStack.pop_back();
        delete codeGenBlock;
    }
    // 添加函数返回值
    void setCurrentRetValue(Value* value){
        blockStack.back()->RetValue = value;
    }
    // 得到函数的返回值
    Value* getCurrentRetValue(){
        return blockStack.back()->RetValue;
    }
    // 打印符号表
    void PrintSymTable() const{
        cout << "#############################符号表#################################" << endl;
        string prefix = "";
        // 从基本块中查找各个作用域-----打印出当前可用变量
        for(auto it=blockStack.begin(); it!=blockStack.end(); it++){
            for(auto it2=(*it)->SymTable.begin(); it2!=(*it)->SymTable.end(); it2++){
                cout << prefix << it2->first << " = " << it2->second << ": "<< this->getSymbolType(it2->first) << endl;
            }
            prefix += "\t";
        }
        cout << "#####################################################################" << endl;
    }
    // 语义分析程序入口
    void generateCode(WangBlock& );
};

Value* LogErrorV(const char* str);
Value* LogErrorV(string str);