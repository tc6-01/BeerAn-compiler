#include "TypeSystem.h"
#include "CodeGen.h"
string TypeSystem::llvmTypeToStr(Type *value) {
    Type::TypeID typeID;
    if( value )
        typeID = value->getTypeID();
    else
        return "Value is nullptr";

    switch (typeID){
        case Type::VoidTyID:
            return "VoidTyID";
        case Type::HalfTyID:
            return "HalfTyID";
        case Type::FloatTyID:
            return "FloatTyID";
        case Type::DoubleTyID:
            return "DoubleTyID";
        case Type::IntegerTyID:
            return "IntegerTyID";
        case Type::FunctionTyID:
            return "FunctionTyID";
        case Type::StructTyID:
            return "StructTyID";
        case Type::ArrayTyID:
            return "ArrayTyID";
        case Type::PointerTyID:
            return "PointerTyID";
        case Type::VectorTyID:
            return "VectorTyID";
        default:
            return "Unknown";
    }
}


string TypeSystem::llvmTypeToStr(Value *value) {
    if( value )
        return llvmTypeToStr(value->getType());
    else
        return "Value is nullptr";
}

TypeSystem::TypeSystem(LLVMContext &context): llvmContext(context){
    addCast(intTy, floatTy, llvm::CastInst::SIToFP);
    addCast(intTy, doubleTy, llvm::CastInst::SIToFP);
    addCast(boolTy, doubleTy, llvm::CastInst::SIToFP);
    addCast(floatTy, doubleTy, llvm::CastInst::FPExt);
    addCast(floatTy, intTy, llvm::CastInst::FPToSI);
    addCast(doubleTy, intTy, llvm::CastInst::FPToSI);
    addCast(intTy, intTy, llvm::CastInst::SExt);
}

Type *TypeSystem::getVarType(const WangIdentifier& type) {
    assert(type.isType);

    return getVarType(type.name);

    return 0;
}

Value* TypeSystem::getDefaultValue(string typeStr, LLVMContext &context) {
    Type* type = this->getVarType(typeStr);
    if( type == this->intTy ){
        return ConstantInt::get(type, 0, true);
    }else if( type == this->doubleTy || type == this->floatTy ){
        return ConstantFP::get(type, 0);
    }
    return nullptr;
}

void TypeSystem::addCast(Type *from, Type *to, CastInst::CastOps op) {
    if( _castTable.find(from) == _castTable.end() ){
        _castTable[from] = std::map<Type*, CastInst::CastOps>();
    }
    _castTable[from][to] = op;
}

Value* TypeSystem::cast(Value *value, Type *type, BasicBlock *block) {
    Type* from = value->getType();
    if( from == type )
        return value;
    if( _castTable.find(from) == _castTable.end() ){
        LogError("Type has no cast");
        return value;
    }
    if( _castTable[from].find(type) == _castTable[from].end() ){
        string error = "Unable to cast from ";
        error += llvmTypeToStr(from) + " to " + llvmTypeToStr(type);
        LogError(error.c_str());
        return value;
    }

    return CastInst::Create(_castTable[from][type], value, type, "cast", block);
}

Type *TypeSystem::getVarType(string typeStr) {

    if( typeStr.compare("int") == 0 ){
        return this->intTy;
    }
    if( typeStr.compare("float") == 0 ){
        return this->floatTy;
    }
    if( typeStr.compare("double") == 0 ){
        return this->doubleTy;
    }
    if( typeStr.compare("bool") == 0 ){
        return this->boolTy;
    }
    if( typeStr.compare("char") == 0 ){
        return this->charTy;
    }
    if( typeStr.compare("void") == 0 ){
        return this->voidTy;
    }
    if( typeStr.compare("string") == 0 ){
        return this->stringTy;
    }
    return nullptr;
}

