#include "Piroof.h"
using namespace Piroof;
using namespace std;

#define token_error(token) Pir_raise("InterpreterError: Unknown token \""+(token)+"\"");

static vart add_number(vart a, vart b) {
	return Pir_copyvar(a);
}

static vart number_add_number(vart a,vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_number(va->val+vb->val);
}
static vart sub_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	return Pir_number(-va->val);
}
static vart number_sub_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_number(va->val - vb->val);
}
static vart number_mul_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_number(va->val * vb->val);
}
static vart number_div_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_number(va->val / vb->val);
}
static vart number_pow_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	if (vb->val.IsFloat())return Pir_expr(Expr(POW,va->val,vb->val));
	if(vb->val.ToDouble()*log10(va->val.ToDouble())>=10.0)
		return Pir_expr(Expr(POW, va->val, vb->val));
	return Pir_number(fastpow(va->val,vb->val.P()));
}
static vart number_eq_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_logic(va->val == vb->val);
}
static vart number_gt_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_logic(va->val > vb->val);
}
static vart number_lt_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_logic(va->val < vb->val);
}
static vart number_ge_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_logic(va->val >= vb->val);
}
static vart number_le_number(vart a, vart b) {
	PirNumberObject* va = (PirNumberObject*)a;
	PirNumberObject* vb = (PirNumberObject*)b;
	return Pir_logic(va->val <= vb->val);
}


static vart expr_add_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(*va->expr + *vb->expr);
}
static vart sub_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	return Pir_expr(-*va->expr);
}
static vart expr_sub_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(*va->expr - *vb->expr);
}
static vart expr_mul_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(*va->expr * *vb->expr);
}
static vart expr_div_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(*va->expr / *vb->expr);
}
static vart expr_pow_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(*va->expr ^ *vb->expr);
}
static vart expr_eq_expr(vart a, vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(*va->expr + *vb->expr);
}
template<int8 tk>
vart expr_op_expr(vart a,vart b) {
	PirExprObject* va = (PirExprObject*)a;
	PirExprObject* vb = (PirExprObject*)b;
	return Pir_expr(Expr(tk,*va->expr,*vb->expr));
}
template<int8 tk>
vart op_expr(vart a,vart b) {
	PirExprObject* va = (PirExprObject*)a;
	return Pir_expr(Expr(tk, *va->expr));
}

static vart string_add_string(vart a, vart b) {
	PirStringObject* va = (PirStringObject*)a;
	PirStringObject* vb = (PirStringObject*)b;
	return Pir_string(va->val+vb->val);
}

void Interpreter::loadOperatorCallbacks() {
	opcallbacks.insert({ "+number",add_number });
	opcallbacks.insert({"number+number",number_add_number});
	opcallbacks.insert({ "-number",sub_number });
	opcallbacks.insert({ "number-number",number_sub_number });
	opcallbacks.insert({ "number*number",number_mul_number });
	opcallbacks.insert({ "number/number",number_div_number });
	opcallbacks.insert({ "number^number",number_pow_number });
	opcallbacks.insert({ "number=number",number_eq_number });
	opcallbacks.insert({ "number>number",number_gt_number });
	opcallbacks.insert({ "number<number",number_lt_number });
	opcallbacks.insert({ "number>=number",number_ge_number });
	opcallbacks.insert({ "number<=number",number_le_number });

	opcallbacks.insert({ "+expr",add_number });
	//opcallbacks.insert({ "expr+expr",expr_add_expr });
	opcallbacks.insert({ "expr+expr",expr_op_expr<ADD> });
	opcallbacks.insert({"-expr",sub_expr});
	opcallbacks.insert({ "expr-expr",expr_sub_expr });
	opcallbacks.insert({ "expr*expr",expr_mul_expr });
	opcallbacks.insert({ "expr/expr",expr_div_expr });
	opcallbacks.insert({ "expr^expr",expr_pow_expr });
	opcallbacks.insert({ "expr=expr",expr_op_expr<EQ> });
	opcallbacks.insert({ "expr>expr",expr_op_expr<GT> });
	opcallbacks.insert({ "expr<expr",expr_op_expr<LT> });
	opcallbacks.insert({ "expr>=expr",expr_op_expr<GE> });
	opcallbacks.insert({ "expr<=expr",expr_op_expr<LE> });
	opcallbacks.insert({ "¬expr",op_expr<NOT> });
	//opcallbacks.insert({ "expr==expr",expr_op_expr<CMP_EQ> });

	opcallbacks.insert({ "string+string",string_add_string });
}
vart Interpreter::operate(const String& op, vart a, vart b) {
	if (!a) {
		Pir_raise("Null object while operating");
	}
	typet ta = Pir_type(a);
	if (tkmp.find(op.c_str()) == tkmp.end()) {
		token_error(op);
		return 0;
	}
	Token tk = tkmp[op.c_str()];
	std::string fname; bool raise=false;
	if (b) {
		typet tb = Pir_type(b);
		if ((vart)ta == PirExprType && (vart)tb != PirExprType) {
			vart tmp = Pir_expr(b);
			vart ret=operate(op, a, tmp);
			Pir_release(tmp);
			return ret;
		}
		if ((vart)ta != PirExprType && (vart)tb == PirExprType) {
			vart tmp = Pir_expr(a);
			vart ret = operate(op, tmp, b);
			Pir_release(tmp);
			return ret;
		}
		fname = (ta->name + op + tb->name).c_str();
	}
	else if (operators[tk].p & OP_POST)
		fname = (ta->name + op).c_str();
	else if (operators[tk].p & OP_PRE)
		fname = (op + ta->name).c_str();
	else raise = true;
	if (raise||opcallbacks.find(fname) == opcallbacks.end()||!opcallbacks[fname]) {
		if (!raise && (vart)ta == PirExprType) {
			Expr& ea = *((PirExprObject*)a)->expr;
			return b ? Pir_expr(Expr(tk,ea, *((PirExprObject*)b)->expr)):Pir_expr(Expr(tk,ea));
		}
		Pir_raise("Cannot find opcallback function \""+fname+"\"");
		return 0;
	}
	return opcallbacks[fname](a,b);
}