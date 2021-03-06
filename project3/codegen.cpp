#include "codegen.hpp"

LabelManager lm;
LabelStruct::LabelStruct(int lc,int max){
	LC = lc;
	Max = max;
	FOR_FLAG = -1;
}

LabelManager::LabelManager(){
	labelCount = 0;
}

void LabelManager::pushNLabel(int n){
	lmStack.push(LabelStruct(labelCount,n));
	labelCount += n;
}
void LabelManager::popLabel(){
	lmStack.pop();
}

int LabelManager::takeLabel(int i){
	if(i >= lmStack.top().Max){
		throw string("Label count out of range!");
	}
	return lmStack.top().LC + i;
}

int LabelManager::getLable(){
	labelCount++;
	return labelCount-1;
}
void LabelManager::addFLAG(){
	lmStack.top().FOR_FLAG = lmStack.top().FOR_FLAG + 1;
}
int LabelManager::getFLAG(){
	return lmStack.top().FOR_FLAG;
}


void genProgramStart(){
	out << "class " << 	outName << endl;
	out << "{" << endl;
}


// function
void genMainStart(){
	out << "method public static void main(java.lang.String[])\n";
	out << "max_stack 15\n";
	out << "max_locals 15\n{\n";
}
void genFuncStart(idInfo info){
	out << "method public static ";
	out << ((info.type == Void_type)? "void":"int");
	out << " " + info.name + "(";
	for (int i = 0; i < info.value.aval.size(); i++) {
		if(i!=0) out << ",";
		out << "int";
	}
	out << ")\n";
	out << "max_stack 15\n";
	out << "max_locals 15\n{\n";
	for (int i = 0; i < info.value.aval.size(); i++) {
		out << "iload " << i << "\n";
	}
}
void genVoidFuncEnd(){
	out << "return\n}\n";
}
void genCompoundEnd(){
	out << "}" << endl;
}

void genCallFunc(idInfo info){
	out << "invokestatic ";
	out << ((info.type == Void_type)? "void":"int");
	out << " " + outName + "." + info.name + "(";
	for (int i = 0; i < info.value.aval.size(); i++) {
		if(i!=0) out << ",";
		out << "int";
	}
	out << ")\n";
}

void genGlobalVar(string name, int value){
	out << "field static int " << name << " = " << value << "\n";
}

void genGlobalVarNoInit(string name){
	out << "field static int " << name << "\n";
}

void genLocalVar(int index,int value){
	out << "ldc " << value << "\nistore " << index << "\n";
}


// print
void genPrintStart(){
	out << "getstatic java.io.PrintStream java.lang.System.out\n";
}
void genPrintStr(){
	out << "invokevirtual void java.io.PrintStream.print(java.lang.String)\n";
}
void genPrintInt(){
	out << "invokevirtual void java.io.PrintStream.print(int)\n";
}
void genPrintlnStr(){
	out << "invokevirtual void java.io.PrintStream.println(java.lang.String)\n";
}
void genPrintlnInt(){
	out << "invokevirtual void java.io.PrintStream.println(int)\n";
}

void genConstStr(string s){
	out << "ldc \"" << s << "\"\n";
}
void genConstInt(int v){
	out << "ldc " << v << "\n";
}

// GET
void genGetGlobalVar(string s){
	out << "getstatic int " << outName << "." << s << "\n";
}
void genGetLocalVar(int index){
	out << "iload " << index << "\n";
}
// SET
void genSetGlobalVar(string s){
	out << "putstatic int " << outName << "." << s << "\n";
}
void genSetLocalVar(int index){
	out << "istore " << index << "\n";
}

void genOperator(char op){
	switch (op) {
		case '+':
			out << "iadd\n";
			break;
		case '-':
			out << "isub\n";
			break;
		case '*':
			out << "imul\n";
			break;
		case '/':
			out << "idiv\n";
			break;
		case '%':
			out << "irem\n";
			break;
		case '&':
			out << "iand\n";
			break;
		case '|':
			out << "ior\n";
			break;
		case '!':
			out << "ldc 1\n";
			out << "ixor\n";
			break;
		case '_':
			out << "ineg\n";
			break;
		default:
			break;
	}
}

void genCondOp(int op){
	out << "isub\n";
	int lb1 = lm.getLable();
	int lb2 = lm.getLable();
	switch (op) {
		case IFLT:
			out << "iflt";
			break;
		case IFGT:
			out << "ifgt";
			break;
		case IFLE:
			out << "ifle";
			break;
		case IFGE:
			out << "ifge";
			break;
		case IFEQ:
			out << "ifeq";
			break;
		case IFNE:
			out << "ifne";
			break;
		default:
			out << "ifeq";
			break;
	}
	out << " L" << lb1 << "\n";
	out << "iconst_0\n";
	out << "goto L" << lb2 << "\n";
	out << "nop\nL" << lb1 << ":\n";
	out << "iconst_1\n";
	out << "nop\nL" << lb2 << ":\n";
}

void genIfStart(){
	lm.pushNLabel(2);
	out << "ifeq L" << lm.takeLabel(0) << "\n";
}
void genElse(){
	out << "goto L" << lm.takeLabel(1) << "\n";
	out << "nop\nL" << lm.takeLabel(0) << ":\n";
}
void genIfEnd(){
	out << "nop\nL" << lm.takeLabel(0) << ":\n";
	lm.popLabel();
}
void genIfElseEnd(){
	out << "nop\nL" << lm.takeLabel(1) << ":\n";
	lm.popLabel();
}

// forloop start for "for(;true;) or for(true;)"
void genForStart(){
	lm.pushNLabel(5);
	lm.addFLAG();
	out << "nop\nL" << lm.takeLabel(0) << ":\n";			// Lstart
}
// forloop start for "for(i=1;true;)"
void genForStart2(){
	lm.addFLAG();
	out << "nop\nL" << lm.takeLabel(0+lm.getFLAG()) << ":\n";		// Lstart2
}
void genForCond(){
	out << "ifeq L" << lm.takeLabel(3+lm.getFLAG()) << "\n";		// if false goto Lexit
	out << "goto L" << lm.takeLabel(2+lm.getFLAG()) << "\n";		// goto Lbody
	out << "nop\nL" << lm.takeLabel(1+lm.getFLAG()) << ":\n";		// Lpost
}
void genForBody(){
	out << "goto L" << lm.takeLabel(0+lm.getFLAG()) << "\n";		// goto Lstart or start2
	out << "nop\nL" << lm.takeLabel(2+lm.getFLAG()) << ":\n";		// Lbody
}
void genForEnd(){
	out << "goto L" << lm.takeLabel(1+lm.getFLAG()) << "\n";		// goto Lpost
	out << "nop\nL" << lm.takeLabel(3+lm.getFLAG()) << ":\n";		// Lexit
	lm.popLabel();
}


void genReturn(){
	out << "return\n";
}
void geniReturn(){
	out << "ireturn\n";
}
