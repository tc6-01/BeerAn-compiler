#include "ASTNodes.h"
#include "CodeGen.h"


extern shared_ptr<WangBlock> programBlock;
extern int yyparse();


int main(int argc, char **argv) {
    cout<<"####################################词法分析########################################"<<endl;
    yyparse();
    cout<<"#####################################抽象语法树##################################"<<endl;
    programBlock->print("--");

    CodeGenContext context;

    context.generateCode(*programBlock);

    return 0;
}