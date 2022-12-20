#ifndef __ASTNODES_H__
#define __ASTNODES_H__
// 建立语法分析节点
// 同时增加语义分析接口用于中间代码生成
#include <llvm/IR/Value.h>
#include <iostream>
#include <vector>

#include <memory>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::shared_ptr;
using std::make_shared;

class CodeGenContext;
class WangBlock;
class WangStatement;
class WangExpression;
class WangVarDel;

typedef std::vector<shared_ptr<WangStatement>> StatementList;
typedef std::vector<shared_ptr<WangExpression>> ExpressionList;
typedef std::vector<shared_ptr<WangVarDel>> VariableList;

// 基类节点
class Node {
public:
    // 打印语法树使用的两个符号
	const char m_DELIM = ':';
	const char* m_PREFIX = "--";
	virtual ~Node() {}
	// 获取当前节点类型---用于后续语法树输出
	virtual string getTypeName() const = 0;
	// 语法树输出
	virtual void print(string prefix) const{}
	// 语义分析接口---中间代码生成
	virtual llvm::Value *codeGen(CodeGenContext &context) { return (llvm::Value *)0; }
};

// 放置多条语句基本块儿的节点
class WangExpression : public Node {
public:
	string getTypeName() const override {
		return "WangExpression";
	}
    virtual void print(string prefix) const override{
        cout << prefix << getTypeName() << endl;
    }
};

// 语句节点
class WangStatement : public Node {
public:
 	string getTypeName() const override {
		return "WangStatement";
	}
    virtual void print(string prefix) const override{
        cout << prefix << getTypeName() << endl;
    }
};

// 程序块节点
class WangBlock : public WangExpression {
public:
    // 语句列表存放基本块儿中的语句
	shared_ptr<StatementList> statements = make_shared<StatementList>();
	string getTypeName() const override {
		return "WangBlock";
	}

	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
        // 遍历所有语句
		for(auto it=statements->begin(); it!=statements->end(); it++){
			(*it)->print(nextPrefix);
		}
	}
	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// double型节点
class WangDouble : public WangExpression {
public:
	double value;
	// 构造函数，进行赋值
	WangDouble(double value): value(value) {}

	string getTypeName() const override {
		return "WangDouble";
	}

	void print(string prefix) const override{
		cout << prefix << getTypeName() << this->m_DELIM << value << endl;
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// int 型节点
class WangInteger : public WangExpression {
public:
    uint64_t value;
	// 与double型节点一样进行赋值
    WangInteger(uint64_t value) : value(value) {}

    string getTypeName() const override {
        return "WangInteger";
    }
    
	void print(string prefix) const override{
        cout << prefix << getTypeName() << this->m_DELIM << value << endl;
    }

    virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 变量名与函数名节点
class WangIdentifier : public WangExpression {
public:
	std::string name;
	WangIdentifier(const std::string &name): name(name) {}

	string getTypeName() const override {
		return "WangIdentifier";
	}

	void print(string prefix) const override{
		cout << prefix << getTypeName() << this->m_DELIM << name << endl;
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 算术操作节点
class WangBinOp : public WangExpression {
public:
    // 操作符
	int op;
    // 左操作数
	shared_ptr<WangExpression> lhs;
    // 右操作数
	shared_ptr<WangExpression> rhs;

    WangBinOp(shared_ptr<WangExpression> lhs, int op, shared_ptr<WangExpression> rhs): lhs(lhs), rhs(rhs), op(op) {    }

	string getTypeName() const override {
		return "WangBinOp";
	}

	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << op << endl;
		lhs->print(nextPrefix);
		rhs->print(nextPrefix);
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 赋值语句节点
class WangAssignment : public WangExpression {
public:
	shared_ptr<WangIdentifier> lhs;
	shared_ptr<WangExpression> rhs;

	WangAssignment(shared_ptr<WangIdentifier> lhs, shared_ptr<WangExpression> rhs): lhs(lhs), rhs(rhs) {}

	string getTypeName() const override {
		return "WangAssignment";
	}

	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		lhs->print(nextPrefix);
		rhs->print(nextPrefix);
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 表达式节点
class WangExpressionStatement : public WangStatement {
public:
	shared_ptr<WangExpression> expression;
	WangExpressionStatement(shared_ptr<WangExpression> expression): expression(expression) {}

	string getTypeName() const override {
		return "WangExpressionStatement";
	}

	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		expression->print(nextPrefix);
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 变量定义节点
class WangVarDel : public WangStatement {
public:
	const shared_ptr<WangIdentifier> type;
	shared_ptr<WangIdentifier> id;
	shared_ptr<WangExpression> assignmentExpr = nullptr;
	WangVarDel(const shared_ptr<WangIdentifier> type, shared_ptr<WangIdentifier> id, shared_ptr<WangExpression> assignmentExpr = NULL)
		: type(type), id(id), assignmentExpr(assignmentExpr) {}

	string getTypeName() const override {
		return "WangVarDel";
	}
	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		type->print(nextPrefix);
		id->print(nextPrefix);
        if( assignmentExpr != nullptr ){
            assignmentExpr->print(nextPrefix);
        }
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 函数定义节点
class WangFuncDel : public WangStatement {
public:
	shared_ptr<WangIdentifier> type;
    shared_ptr<WangIdentifier> id;
	shared_ptr<VariableList> arguments = make_shared<VariableList>();
	shared_ptr<WangBlock> block;

	WangFuncDel(shared_ptr<WangIdentifier> type, shared_ptr<WangIdentifier> id, shared_ptr<VariableList> arguments, shared_ptr<WangBlock> block)
		: type(type), id(id), arguments(arguments), block(block) {}

	string getTypeName() const override {
		return "WangFuncDel";
	}

	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;

		type->print(nextPrefix);
		id->print(nextPrefix);

		for(auto it=arguments->begin(); it!=arguments->end(); it++){
			(*it)->print(nextPrefix);
		}
        if( block )
		    block->print(nextPrefix);
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// 返回值节点
class WangReturn: public WangStatement{
public:
    shared_ptr<WangExpression> expression;

    WangReturn(shared_ptr<WangExpression>  expression): expression(expression) {}

    string getTypeName() const override {
        return "WangReturn";
    }

    void print(string prefix) const override {
        string nextPrefix = prefix + this->m_PREFIX;
        cout << prefix << getTypeName() << this->m_DELIM << endl;

        expression->print(nextPrefix);
    }

    virtual llvm::Value* codeGen(CodeGenContext& context) override ;

};

// 调用函数节点
class WangCallFunc: public WangExpression {
public:
	const shared_ptr<WangIdentifier> id;
	shared_ptr<ExpressionList> arguments = make_shared<ExpressionList>();
	// 有参数的函数调用
	WangCallFunc(const shared_ptr<WangIdentifier> id, shared_ptr<ExpressionList> arguments): id(id), arguments(arguments) {}
	// 无参数函数调用
	WangCallFunc(const shared_ptr<WangIdentifier> id): id(id) {}

	string getTypeName() const override {
		return "WangCallFunc";
	}

	void print(string prefix) const override{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		this->id->print(nextPrefix);
		for(auto it=arguments->begin(); it!=arguments->end(); it++){
			(*it)->print(nextPrefix);
		}
	}

	virtual llvm::Value* codeGen(CodeGenContext& context) override ;
};

// if语句节点
class WangIf: public WangStatement{
public:
    shared_ptr<WangExpression>  condition;
    shared_ptr<WangBlock> trueBlock;       
    shared_ptr<WangBlock> falseBlock;      

    WangIf(shared_ptr<WangExpression>  cond, shared_ptr<WangBlock> blk, shared_ptr<WangBlock> blk2 = nullptr)
            : condition(cond), trueBlock(blk), falseBlock(blk2){
    }
    string getTypeName() const override {
        return "WangIf";
    }

    void print(string prefix) const override{
        string nextPrefix = prefix + this->m_PREFIX;
        cout << prefix << getTypeName() << this->m_DELIM << endl;
        condition->print(nextPrefix);
        trueBlock->print(nextPrefix);
        if( falseBlock ){
            falseBlock->print(nextPrefix);
        }
    }
    llvm::Value *codeGen(CodeGenContext &context) override ;
};

// for语句节点
class WangFOR: public WangStatement{
public:
    shared_ptr<WangExpression> initial, condition, increment;
    shared_ptr<WangBlock>  block;
    WangFOR(shared_ptr<WangBlock> b, shared_ptr<WangExpression> init = nullptr, shared_ptr<WangExpression> cond = nullptr, shared_ptr<WangExpression> incre = nullptr)
            : block(b), initial(init), condition(cond), increment(incre){ }
    string getTypeName() const override{
        return "WangFOR";
    }
    void print(string prefix) const override{
        string nextPrefix = prefix + this->m_PREFIX;
        cout << prefix << getTypeName() << this->m_DELIM << endl;

        if( initial )
            initial->print(nextPrefix);
        if( condition )
            condition->print(nextPrefix);
        if( increment )
            increment->print(nextPrefix);
        block->print(nextPrefix);
    }
    llvm::Value *codeGen(CodeGenContext &context) override ;

};

std::unique_ptr<WangExpression> LogError(const char* str);

#endif
