#include "ASTNodes.h"
#include "CodeGen.h"


extern shared_ptr<WangBlock> programBlock;
extern int yyparse();


int main(int argc, char **argv) {
    cout<<"####################################词法分析########################################"<<endl;
    cout<<"line"<<"\t"<<"symbol"<<endl;
    yyparse();
    cout<<"#####################################抽象语法树##################################"<<endl;
    programBlock->print("--");
    cout<<"##############################语义分析&中间代码生成###############################"<<endl;
    CodeGenContext context;
    context.generateCode(*programBlock);

    return 0;
}