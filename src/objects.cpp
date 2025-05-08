#include "Piroof.h"
using namespace std;
using namespace Piroof;

#define convert_error(a, b) Pir_raise("Cannot convert \"" + a + "\" to \""+b+"\"")

vart Piroof::Pir_new(vart type, uint8 state) {
	vart ret = (vart)Pir_MemoryPool.allocate(((typet)type)->size);
	ret->type = type;
	ret->state = state;
	return ret;
}
void Piroof::Pir_forceRelease(vart var) {
	//cout << "force release "; Pir_print(var);
	if (!var || var->state & ST_DEL)return;
	var->state |= ST_DEL;
	typet type = Pir_type(var);
	if (type && type->_release)
		type->_release(var);
	if (var->attr) {
		try {
			delete var->attr;
		}
		catch (...) {}
	}
	Pir_MemoryPool.deallocate(var, Pir_sizeof(var));
}
void Piroof::Pir_release(vart var) {
	//cout << "releasing "; Pir_print(var);
	if (!var || var->state & ST_DEL ||var->state&ST_SHARED)return;
	var->state |= ST_DEL;
	typet type = Pir_type(var);
	if (type && type->_release)
		if(type->_release(var))return;
	if (var->attr) {
		try {
			delete var->attr;
		}
		catch (...) {}
	}
	Pir_MemoryPool.deallocate(var, Pir_sizeof(var));
}

size_t Piroof::Pir_sizeof(vart obj) {
	return Pir_type(obj)->size;
}
typet Piroof::Pir_type(vart obj) {
	if (!obj) {
		Pir_raise("Null object");
		return 0;
	}
	return (typet)obj->type;
}
void Piroof::Pir_exit(int code) {
	exit(code);
}

vart Piroof::Pir_exception(vart type, const String& msg) {
	PirExceptionObject* ret = (PirExceptionObject*)Pir_new(type);
	ret->msg = msg;
	return (vart)ret;
}
void Piroof::Pir_throw(vart exc) {
	PirExceptionObject* x = (PirExceptionObject*)exc;
	typet type = Pir_type(exc);
	throw (type->name + ": " + x->msg).c_str();
	//cerr << type->name + ": " + x->msg << endl;

}
void Piroof::Pir_error(const String& msg) {
	cerr << "Error: " << msg << endl;
	exit(1);
}
void Piroof::Pir_raise(const String& msg) {
	Pir_throw(Pir_exception(PirExceptionType, msg));
}

vart Piroof::Pir_tostr(vart obj) {
	typet type = Pir_type(obj);
	if (!obj||!type) {
		Pir_raise("Null object");
		return 0;
	}
	if (obj->attr&&obj->attr->find("__string__")!=obj->attr->end()) {
		return Pir_call((*obj->attr)["__string__"], Arguments());
	}
	if (!type->_str) {
		Pir_raise("The object has no method \"__string__\"");
		return 0;
	}
	return type->_str(obj);
}

vart Piroof::Pir_string(const String& s) {
	PirStringObject* ret = (PirStringObject*)Pir_new(PirStringType);
	ret->val = s;
	return (vart)ret;
}

void Piroof::Pir_print(vart obj) {
	PirStringObject* s = (PirStringObject*)Pir_tostr(obj);
	printf(s->val.c_str());
	printf("\n");
}

vart ObjectFunc::operator()(vart obj, const Arguments& args)const {
	std::string msg = check(args);
	if (msg.size())Pir_raise(msg);
	for (const auto& i : p) {
		if (i.x && !args.find(i.str))
			args._insert(i.str,i.x);
	}
	if (!f) {
		Pir_raise("Null object has been called");
		return 0;
	}
	return f(obj, args);
}

vart Piroof::Pir_call(vart obj,const Arguments& args) {
	typet type = Pir_type(obj);
	if (!type) {
		Pir_raise("Null object has been called");
		return 0;
	}
	if (obj->attr&&obj->attr->find("__call__")!=obj->attr->end()) {
		return Pir_call((*obj->attr)["__call__"], args);
	}
	if (!type->_call) {
		Pir_raise("The object has no method \"__call__\"");
		return 0;
	}
	return (type->_call)(obj, args);
}
vart Piroof::Pir_copyvar(vart obj) {
	vart ret;
	if(Pir_type(obj)->_new)ret = Pir_type(obj)->_new(obj);
	else {
		Pir_raise("The object has no method \"__new__\"");
		return 0;
	}
	ret->state = obj->state;
	return ret;
}
vart Piroof::Pir_copybytes(vart obj, uint8 state) {
	typet type = Pir_type(obj);
	vart ret = Pir_new((vart)type);
	for (size_t i = 0; i < type->size; i++)
		((uint8*)ret)[i] = ((uint8*)obj)[i];
	ret->state = state;
	return ret;
}
vart Piroof::Pir_getatttr(vart obj, const char* a) {
	if (!obj || !obj->attr) {
		Pir_raise("The object has no attributes");
		return 0;
	}
	return (*obj->attr)[a];
}

static vart type_str(vart obj) {
	return Pir_string(((PirTypeObject*)obj)->name);
}
static vart type_new(vart obj) {
	return (vart)obj->type;
}
static vart type_call(vart obj, const Arguments& args) {
	return ((typet)obj)->_new(args[0]);
}
PirTypeObject _PirTypeType = {
	PirTypeType,ST_CONST,0,ObjectFunc(type_call,{{"0",0}}),
	sizeof(PirTypeObject),"type",
	type_str,type_new,0,
};
vart Piroof::PirTypeType = (vart)&_PirTypeType;

static vart string_str(vart obj) {
	return obj;
}
static vart string_new(vart obj) {
	return 0;
}
PirTypeObject _PirStringType = {
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirStringObject),"string",
	string_str,string_new
};
vart Piroof::PirStringType = (vart)&_PirStringType;

static vart Exception_str(vart obj) {
	typet type = Pir_type(obj);
	return Pir_string(type->name + "(\"" + ((PirExceptionObject*)obj)->msg + "\")");
}
PirTypeObject _PirExceptionType = {
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirExceptionObject),"Exception",
	Exception_str
};
vart Piroof::PirExceptionType = (vart)&_PirExceptionType;

static vart void_str(vart obj) {
	return Pir_string("void");
}
static vart void_new(vart obj) {
	return PirVoid;
}
PirTypeObject _PirVoidType{
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirVoidObject),"voidtype",
	void_str,void_new
};
vart Piroof::PirVoidType = (vart)&_PirVoidType;
vart Piroof::PirVoid = Pir_new(PirVoidType);

static vart BF_str(vart obj) {
	return Pir_string("<bulitin-function>");
}
static vart BF_call(vart obj, const Arguments& args) {
	return ((PirBFObject*)obj)->f(obj, args);
}
PirTypeObject _PirBFType{
	PirTypeType,ST_CONST,0,ObjectFunc(BF_call),
	sizeof(PirBFType),"type\"bulitin-function\"",
	BF_str
};
vart Piroof::PirBFType = (vart)&_PirBFType;


static vart Pir_print_f(vart obj,const Arguments& args) {
	PirStringObject* s = (PirStringObject*)Pir_tostr(args[0]);
	PirStringObject* end = (PirStringObject*)Pir_tostr(args[1]);
	printf(s->val.c_str());
	printf(end->val.c_str());
	Pir_release((vart)s);
	Pir_release((vart)end);
	return PirVoid;
}
PirStringObject _Pirendl = {
	PirStringType,ST_CONST,0,"\n"
};
vart Pirendl = (vart)&_Pirendl;
PirBFObject Pir_print_BF = {
	PirBFType,ST_CONST | ST_ARGC,0,
	ObjectFunc(Pir_print_f,{{"obj",0},{"end",Pirendl}})
};
vart Piroof::PirPrint = (vart)&Pir_print_BF;

static vart Pir_exit_f(vart obj, const Arguments& args) {
	typet type = Pir_type(args[0]);
	if ((vart)type != PirIntType) {
		Pir_raise("Type of the exit code should be int");
		return PirVoid;
	}
	exit(((PirIntObject*)args[0])->val);
	return PirVoid;
}
PirBFObject Pir_exit_BF = {
	PirBFType,ST_CONST | ST_ARGC,0,
	ObjectFunc(Pir_exit_f,{{"code",0}})
};
vart Piroof::PirExit = (vart)&Pir_exit_BF;

static vart Pir_isconst_f(vart obj, const Arguments& args) {
	return Pir_logic(args[0]->state & ST_CONST);
}
PirBFObject Pir_isconst_BF = {
	PirBFType,ST_CONST | ST_ARGC,0,
	ObjectFunc(Pir_isconst_f,{{"obj",0}})
};
vart Piroof::PirIsconst = (vart)&Pir_isconst_BF;


static vart logic_str(vart obj) {
	return Pir_string(((PirLogicObject*)obj)->val.toString());
}
PirTypeObject _PirLogicType{
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirLogicObject),"logic",
	logic_str
};
vart Piroof::PirLogicType = (vart)&_PirLogicType;
vart Piroof::Pir_logic(LogicState ls) {
	PirLogicObject* ret = (PirLogicObject*)Pir_new(PirLogicType);
	ret->val = ls;
	return (vart)ret;
}

static vart number_str(vart obj) {
	return Pir_string(((PirNumberObject*)obj)->val.ToString<String>());
}
static vart number_new(vart obj) {
	if (obj->type == PirNumberType) {
		return Pir_copybytes(obj,ST_CONST);
	}
	else if (obj->type == PirStringType) {
		return Pir_number(((PirStringObject*)obj)->val.c_str());
	}
	else if (obj->type == PirFloatType) {
		return Pir_number(((PirFloatObject*)obj)->val);
	}
	convert_error(Pir_type(obj)->name, "number");
	return 0;
}
PirTypeObject _PirNumberType={
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirNumberObject),"number",
	number_str,number_new
};
vart Piroof::PirNumberType = (vart)&_PirNumberType;
vart Piroof::Pir_number(const RationalNumber& n) {
	PirNumberObject* ret = (PirNumberObject*)Pir_new(PirNumberType);
	ret->val = n;
	return (vart)ret;
}

static vart float_str(vart obj) {
	return Pir_string(toString(((PirFloatObject*)obj)->val));
}
static vart float_new(vart obj) {
	if (obj->type == PirFloatType) {
		return Pir_copybytes(obj, ST_CONST);
	}
	else if (obj->type == PirNumberType) {
		return Pir_float(((PirNumberObject*)obj)->val.ToDouble());
	}
	else if (obj->type == PirStringType) {
		return Pir_float(stringTo<double>(((PirStringObject*)obj)->val.c_str()));
	}
	convert_error(Pir_type(obj)->name, "float");
	return 0;
}
PirTypeObject _PirFloatType = {
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirTypeObject),"float",
	float_str,float_new
};
vart Piroof::PirFloatType = (vart)&PirFloatType;
vart Piroof::Pir_float(double x) {
	PirFloatObject* ret = (PirFloatObject*)Pir_new(PirFloatType);
	ret->val = x;
	return (vart)ret;
}

static vart int_str(vart obj) {
	return Pir_string(((PirIntObject*)obj)->val.ToString());
}
static vart int_new(vart obj) {
	if (obj->type == PirIntType) {
		return Pir_copybytes(obj, ST_CONST);
	}
	else if (obj->type == PirFloatType) {
		return Pir_int((int64)((PirFloatObject*)obj)->val);
	}
	else if (obj->type == PirNumberType) {
		return Pir_int((int64)((PirNumberObject*)obj)->val.ToDouble());
	}
	else if (obj->type == PirStringType) {
		return Pir_int(((PirStringObject*)obj)->val.c_str());
	}
	convert_error(Pir_type(obj)->name, "float");
	return 0;
}
PirTypeObject _PirIntType = {
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirIntObject),"int",
	int_str,int_new
};
vart Piroof::PirIntType = (vart)&_PirIntType;
vart Piroof::Pir_int(const Integer& x) {
	PirIntObject* ret = (PirIntObject*)Pir_new(PirIntType);
	ret->val = x;
	return (vart)ret;
}

vart Piroof::PirTrue = Pir_logic(true),
Piroof::PirFalse = Pir_logic(false),
Piroof::PirNotsure = Pir_logic(notsure),
Piroof::PirContra = Pir_logic(contra),
Piroof::PirInf = Pir_number(inf),
Piroof::PirInd = Pir_number(ind);


static vart tuple_str(vart obj) {
	PirTupleObject* t = (PirTupleObject*)obj;
	if (t->val.size() > 100)return Pir_string("<tuple>");
	String ret = "[";
	for (size_t i = 0; i < t->val.size(); i++) {
		vart s = Pir_tostr(t->val[i]);
		ret += ((PirStringObject*)s)->val+",";
		Pir_release(s);
	}
	if (t->val.size())ret.back() = ']';
	else ret.push_back(']');
	return Pir_string(ret);
}
static vart tuple_new(vart obj) {
	PirTupleObject* ret = (PirTupleObject*)Pir_new(PirTupleType);
	typet type = Pir_type(obj);
	ret->val = Vector<vart>();
	if ((vart)type == PirTupleType) {
		ret->val = ((PirTupleObject*)obj)->val;
		return (vart)ret;
	}
	ret->val.emplace_back(obj);
	return (vart)ret;
}
PirTypeObject _PirTupleType = {
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirTupleObject),"tuple",
	tuple_str,tuple_new
};
vart Piroof::PirTupleType=(vart)&_PirTupleType;
vart Piroof::Pir_tuple(const std::vector<vart>& t) {
	PirTupleObject* ret = (PirTupleObject*)Pir_new(PirTupleType);
	ret->val = Vector<vart>();
	ret->val.resize(t.size());
	for (size_t i = 0; i < ret->val.size(); i++)ret->val[i] = t[i];
	return (vart)ret;
}