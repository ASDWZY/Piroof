#include <fstream>

#include "number.cpp"
#include "common.cpp"
#include "objects.cpp"
#include "expr.cpp"
#include "calc.cpp"

#define IsDelimiter(x) ((x)==' '||(x)=='\t')
#define IsEOE(x) ((x)==','||(x)==';'||(x)==']'||(x)==')'||(x)=='}')
#define IsNum(x) ('0'<=(x)&&(x)<='9')
#define IsLetter(x) (('A'<=(x)&&(x)<='Z')||('a'<=(x)&&(x)<='z')||(x)=='_')
#define lawful_ch(x) (('0'<=(x)&&(x)<='9')||IsLetter(x))
#define token_error(token) Pir_raise("InterpreterError: Unknown token \""+(token)+"\"");
#define IsNotVar(x) ((x).tk != OBJECT || (x).ch.size() || (x).trash)
#define symboldef_error() Pir_raise("InterpreterError: Unlawful symbol defination")
#define exprfuncdef_error() Pir_raise("InterpreterError: Unlawful expr_function defination")
#define unknownvar_error(name) Pir_raise("InterpreterError: Unknown variable \"" + (name) + "\"")
#define redefine_error(name) Pir_raise("InterpreterError: Var \""+(name)+"\" has been redefined")
#define importform_error() Pir_raise("InterpreterError: Token \"import\" should decorate one string of the path of the file ");

struct BuiltinVars {
	unordered_map<std::string, vart> globals;
	BuiltinVars() {
		globals.insert({ "type", PirTypeType });
		globals.insert({ "logic", PirLogicType });
		globals.insert({ "number", PirNumberType });
		globals.insert({ "int", PirIntType });
		globals.insert({ "float", PirFloatType });
		globals.insert({ "string", PirStringType });
		globals.insert({ "Exception", PirExceptionType });
		globals.insert({ "expr",PirExprType });
		globals.insert({"interpreter", PirInterpreterType});

		globals.insert({ "void", PirVoid });
		globals.insert({ "true", PirTrue });
		globals.insert({ "false", PirFalse });
		globals.insert({ "notsure", PirNotsure });
		globals.insert({ "true", PirContra });
		globals.insert({ "inf", PirInf });
		globals.insert({ "ind", PirInd });

		globals.insert({ "print", PirPrint });
		globals.insert({ "exit", PirExit });
		globals.insert({ "isconst", PirIsconst });

		globals.insert({ "isconstexpr",PirIsconstexpr });
		globals.insert({ "expand", PirExpand });
	}
	~BuiltinVars() {

	}
	void release() {
		globals.clear();
	}
	bool findvar(const std::string& name) {
		return globals.find(name) != globals.end();
	}
};
BuiltinVars PirBV;

void Interpreter::initGlobals() {
	globals = PirBV.globals;
}
void Interpreter::initTkmp() {
	tkmp = {
		{"=",EQ},
		{".",GETATTR},
		{"equals",EQ},

		{"+",ADD},
		{"-",SUB},
		{"*",MUL},
		{"/",DIV},
		{"^",POW},

		{"<",LT},
		{">",GT},
		{"<=",LE},
		{">=",GE},

		{"not",NOT},
		{"or",OR},
		{"and",AND},
		//{",",AND},
		{"=>",IMPLIES},
		{"implies",IMPLIES},
		{"<=>",EQUIVALENT},

		{"symbol",SYMBOL},
		{"const",CONSTANT},

		{"if",IF},
		{"for",FOR},
		{"while",WHILE},
		{"delete",DELETE},
		{"let",LET},
		{"var",VAR},
		{"assign",ASSIGN},
		{"suppose",SUPPOSE},
		{"import",IMPORT},
	};
}
void Interpreter::initOperators() {
	operators = {
		{-1,OP_NONE},//none 0
		{100,OP_PRE},//obj 1

		{100},//string 2
		{100},//fncall 3
		{100},//tuple 4
		{100},//dict 5

		{100,OP_PRE,false,"if "},//if 6
		{100,OP_PRE,false,"for "},//for 7
		{100,OP_PRE,false,"while "},//while 8
		{100,OP_PRE,false,"delete "},//delete 9

		{10,OP_MID | OP_PRE,false,"+"},//add 10
		{10,OP_MID | OP_PRE,false,"-"},//sub 11
		{11,OP_MID,false,"*"},//mul 12
		{11,OP_MID,false,"/"},//div 13
		{12,OP_MID,true,"^"},//pow 14

		{2,OP_MID,true,"="},//= 15
		{100,OP_MID,false,"."},// . 16
		{8,OP_MID,false,"<"},//< 17
		{8,OP_MID,false,">"},//> 18
		{8,OP_MID,false,"<="},//<= 19
		{8,OP_MID,false,">="},//>= 20

		{20,OP_PRE,false,"¬"},//not 
		{5,OP_MID,false," and "},//and 
		{4,OP_MID,false," or "},//or
		{3,OP_MID,false,"=>"},//implies 
		{3,OP_MID,false,"<=>"},//equivalent

		{1,OP_PRE,false,"symbol "},//symbol
		{1,OP_PRE,false,"const "},//const
		{100},//number
		{1,OP_PRE,false,"var "},//var
		{1,OP_PRE,false,"assign "},//assign
		{100},//function

		{1,OP_PRE,false,"let "},//let
		{1,OP_PRE,false,"suppose "},//suppose
		{1,OP_PRE,false},//include

		{-1,OP_NONE}//max_token
	};
}
void Interpreter::clear() {
	for (auto& it : globals) {
		if (it.second&&!PirBV.findvar(it.first))
			Pir_forceRelease(it.second);
	}
}

Interpreter::Interpreter() {
	initGlobals();
	loadOperatorCallbacks();
	initOperators();
	initTkmp();
}
void Interpreter::release() {
	clear();
}
Interpreter::~Interpreter() {
	release();
}


void Interpreter::_Expr::release() {
	for (size_t i = 0; i < ch.size(); i++)
		ch[i].release();
	if (trash && val && val != PirVoid) {
		//std::cout << "del const " << exprstr(*this) << '\n';
		Pir_release(val);
		val = 0;
	}
}

Token Interpreter::token(const char* bg, size_t len) {
	std::string s(bg, len);
	// s->""
	if (tkmp.find(s) != tkmp.end()) {
		Token tk = tkmp[s];
		st.push({ tk,s,0,{},false });// s->""
		return tk;
	}


	if (isNumber(s.c_str())) {
		st.push({ NUMBER,s,Pir_number(s.c_str()),{},true });// s->""
		return NUMBER;
	}
	size_t i = 0;
	while (i < len && (IsNum(s[i]) || (i && i + 1 < len && s[i] == 'e') || s[i] == '.' || s[i] == '-'))i++;
	if (i == len)return NONE;
	_Expr num;
	if (i) {
		std::string nums(bg, i);
		//cout << "nums " << nums << " " << isNumber(nums.c_str()) << endl;
		if (isNumber(nums.c_str()))
			num = { NUMBER,nums,Pir_number(nums.c_str()),{},true };//s->""
		else if (nums == ".") {
			st.push({ GETATTR,".",0,{},false,false });
			fold();
		}
		else return NONE;
	}

	for (size_t j = i; j < len; j++)
		if (!IsLetter(bg[j]) && !IsNum(bg[j]))return NONE;

	std::string sub = s.c_str() + i;
	//cout << "asd " << sub << endl;
	if (globals.find(sub) != globals.end()) {
		_Expr var = { OBJECT,sub,globals[sub],{},true,false };
		if (num.val)st.push({ MUL,"*",0,{num,var},true });//s->""
		else st.push(var);
		return OBJECT;
	}
	//a*b*c=abc
	_Expr var = { OBJECT,sub,0,{},true,false };
	if (num.val)st.push({ MUL,"*",0,{num,var},true });//s->""
	else st.push(var);
	return OBJECT;
}

int Interpreter::push() {
	_Expr lst;
	while (st.size() >= 2 && st.top().comp) {
		lst = st.top(); st.pop();
		_Expr& f = st.top();
		Operator& op = operators[f.tk];
		if (op.m && op.m != OP_POST) {
			f.ch.emplace_back(lst);
			cout << "fold " << lst.tostr() << " -> " << f.tostr() << endl;
			f.comp = true;
		}
		else {
			st.push(lst);
			break;
		}
	}
	return 0;
}

int Interpreter::fold() {
	if (!st.size() || st.top().comp)return 0;
	_Expr tp = st.top(); st.pop();
	_Expr lst;
	//for pre place in op
	while (st.size() >= 2 && st.top().comp) {
		lst = st.top(); st.pop();
		_Expr& f = st.top();
		Operator& op = operators[f.tk];
		if (op.m && op.m != OP_POST && op.p >= operators[tp.tk].p &&
			!(op.post && f.tk == tp.tk)) {
			f.ch.emplace_back(lst);
			//cout << "fold1 " << lst.tostr() << " -> " << f.tostr() <<" | "<<tp.tostr()<< endl;
			f.comp = true;
		}
		else {
			st.push(lst);
			break;
		}
	}
	//for post place in op
	Operator& op = operators[tp.tk];
	if (op.m && op.m != OP_PRE) {
		if (st.top().comp) {
			tp.ch.emplace_back(st.top());
			//cout << "fold2 " << st.top().tostr() << " -> " << tp.tostr() << endl;
			st.pop();
			st.push(tp);
			return 0;
		}
		/*else if (!(op.m & OP_PRE)) {
			Pir_raise("InterpreterError");
			return 1;
		}*/
	}
	st.push(tp);
	return 0;
}

int Interpreter::normalize(_Expr& expr) {
	if (expr.tk == FNCALL && expr.ch.size() == 1) { _Expr t = expr.ch[0]; expr = t; }
	if (expr.ch.size() == 1 && expr.ch[0].tk == FNCALL) {
		if (expr.tk == OBJECT || expr.tk == NUMBER) expr.tk = FNCALL;
		auto t = expr.ch[0].ch;
		expr.ch = t;
	}
	for (size_t i = 0; i < expr.ch.size(); i++) {
		if (normalize(expr.ch[i]))return 1;
	}
	return 0;
}

int Interpreter::checkVar(const _Expr& expr) {
	if (expr.tk == OBJECT && expr.val == 0) {
		unknownvar_error(expr.name);
		return 1;
	}
	return 0;
}

int Interpreter::buildExpr(_Expr& expr, expr_map& mp) {
	if (expr.ch.size()) {
		for (size_t i = 0; i < expr.ch.size(); i++)
			if (buildExpr(expr.ch[i], mp))return 1;
		return 0;
	}
	if (mp.find(expr.name)!=mp.end()) {
		expr.trash = false;
		expr.val = mp[expr.name];
		return 0;
	}
	if (expr.tk == OBJECT && !expr.val) {
		unknownvar_error(expr.name);
		return 1;
	}
	if (expr.tk != OBJECT) {
		if (expr.val) {
			typet type = Pir_type(expr.val);
			if ((vart)type == PirExprType)return 0;
		}
		expr.tk = OBJECT;
		expr.trash = true;
		vart tmp = Pir_expr(expr.val);
		Pir_release(expr.val);
		expr.val = tmp;
		return 0;
	}
	return 0;
}

vart Interpreter::buildFncall(_Expr& f) {
	if (!f.val) {
		Pir_raise("Null object has been called");
		return 0;
	}
	Arguments inp;
	if (!(f.val->state & ST_ARGC)) {
		for (size_t i = 0; i < f.ch.size(); i++)
			inp.append(perform(f.ch[i]));
		//std::cout << "fncallwoa " << f.name << '\n';
		return f.val = Pir_call(f.val, inp);
	}
	std::list<vart> args;
	Arguments kwargs;
	ObjectFunc& fn = ((PirBFObject*)f.val)->f;
	for (size_t i = 0; i < f.ch.size(); i++) {
		if (f.ch[i].tk == EQ) {
			if (f.ch[i].ch.size() != 2) {
				Pir_raise("InterpreterError: Token EQ should with 2 arguments");
				return 0;
			}
			kwargs.insert(f.ch[i].ch[0].name,perform(f.ch[i].ch[1]));
		}
		else {
			args.emplace_back(perform(f.ch[i]));
			//Pir_print(args.back());
		}
		
	}
	for (const auto& i : fn.p.c) {
		if (kwargs.find(i.str)) {
			inp.insert(i.str, kwargs[i.str]);
			kwargs.erase(i.str);
		}
		else if (args.size()) {
			inp.insert(i.str, args.front());
			args.pop_front();
		}
		else if (i.x) {
			inp.insert(i.str, i.x);
		}
		else {
			Pir_raise("InterpreterError: function \""+f.name+
				"\" missing a required argument \""+i.str+"\"");
			return 0;
		}
	}
	//if(fn.p.find("*args"))inp.insert("*args",)
	//std::cout << "fncall " << f.name << '\n';
	return f.val=Pir_call(f.val,inp);
}

std::string Interpreter::getOpname(int tk)const {
	return operators[tk].name;
}

vart Interpreter::createVar(_Expr& expr,Token tk, bool assign,const std::string& tkname) {
	for (size_t i = 0; i < expr.ch.size(); i++) {
		std::string* name; vart y = PirVoid;
		if (expr.ch[i].tk == EQ) {
			if (expr.ch[i].ch.size() != 2 || IsNotVar(expr.ch[i].ch[0])) {
				Pir_raise("InterpreterError: Token \""+tkname+"\" can only decorate a=b or a formed expressions");
				return 0;
			}
			name = &expr.ch[i].ch[0].name;
			y = perform(expr.ch[i].ch[1]);
			expr.ch[i].ch[0].trash = false;
		}
		else {
			if (expr.ch[i].tk!=OBJECT) {
				Pir_raise("InterpreterError: Token \""+tkname+"\" can only decorate a=b or a formed expressions");
				return 0;
			}
			name = &expr.ch[i].name;
		}
		if (!assign&&globals.find(*name) != globals.end()) {
			redefine_error(*name);
			return 0;
		}
		if (assign && globals.find(*name) != globals.end() && globals[*name]->state & ST_CONST) {
			Pir_raise("Constant \""+*name+"\" cannot be assigned");
			return 0;
		}
		if (y) {
			vart x = Pir_copyvar(y);
			x->state = tk == CONSTANT ? ST_CONST : 0;
			vart& g = globals[*name];
			if (assign && g)Pir_release(g);
			g = x;
		}
		else return 0;
		expr.ch[i].trash = false;
	}
	expr.trash = false;
	return PirVoid;
}

vart Interpreter::perform(_Expr& expr,Token tk) {
	std::vector<vart> args;
	if (tk == CONSTANT && expr.tk != SYMBOL) {
		Pir_raise("Cannot use \"const\" here");
		return 0;
	}

	switch (expr.tk) {
	case EQ:
	{
		_Expr& v = expr.ch[0];
		if (v.tk == FNCALL) {
			if (v.val)break;
			//func_expr
			expr_map varmp;
			for (size_t i = 0; i < v.ch.size(); i++) {
				if (IsNotVar(v.ch[i])) {
					exprfuncdef_error();
					return 0;
				}
				if (varmp.find(v.ch[i].name) != varmp.end()) {
					exprfuncdef_error();
					return 0;
				}
				varmp[v.ch[i].name]=Pir_symexpr(v.ch[i].name,VAR,int8(i));
				
			}
			if (buildExpr(expr.ch[1], varmp))return 0;
			vart e = perform(expr.ch[1]);
			if (!e)return 0;
			if (e->type != PirExprType) {
				vart tmp = Pir_expr(e);
				Pir_release(e);e = tmp;
			}
			PirExprObject* t = (PirExprObject*)e;
			if (t->expr->tk == SYMBOL || t->expr->tk == VAR)
				e = Pir_copyvar(e);
			globals[v.name] = e;
			t = (PirExprObject*)e;
			//std::cout << "buliding " << t->expr.serialize(*this) << endl
			t->pos = -2;
			t->numargs = int8(varmp.size());
			//t->expr = new Expr(*(t->expr));
			expr.ch[0].trash = false;
			expr.ch[1].trash = false;
			expr.trash = false;
			return PirVoid;
		}
		if (v.tk != OBJECT)break;
		if (globals.find(v.name) == globals.end()) {
			unknownvar_error(v.name);
			return 0;
		}
		vart y = perform(expr.ch[1]);
		return operate("=", v.val, y);
	}
	case ASSIGN: 
		return createVar(expr,tk,true,"assign");
		
	case VAR: 
		return createVar(expr, tk, false,"var");

	case SYMBOL:
		for (size_t i = 0; i < expr.ch.size(); i++) {
			if (IsNotVar(expr.ch[i])) { symboldef_error(); return 0; }
			if (expr.ch[i].val) { redefine_error(expr.ch[0].name); return 0; }
			globals[expr.ch[i].name] = Pir_symexpr(expr.ch[i].name, tk==CONSTANT?CONSTANT:SYMBOL);
			expr.ch[i].trash = false;
		}
		expr.trash = false;
		return PirVoid;
	case CONSTANT:
		if (expr.ch.size() != 1) {
			Pir_raise("Token \"const\" should decorate one expression");
			return 0;
		}
		if (expr.ch[0].tk == EQ)
			return createVar(expr,CONSTANT,false,"const");
		return perform(expr.ch[0],CONSTANT);
	case DELETE:
		for (size_t i = 0; i < expr.ch.size(); i++) {
			if (IsNotVar(expr.ch[i])) {
				Pir_error("ReferenceError: Cannot delete a non-var object");
				return 0;
			}
			if (!findvar(expr.ch[i].name)) {
				unknownvar_error(expr.ch[i].name);
				return 0;
			}
			if (PirBV.findvar(expr.ch[i].name)) {
				Pir_error("ReferenceError: Cannot delete an inner object");
				return 0;
			}
			globals.erase(expr.ch[i].name);
			Pir_forceRelease(expr.ch[i].val);
		}
		return PirVoid;
	case GETATTR:
		if (expr.ch.size() != 2 || expr.ch[0].tk != OBJECT|| expr.ch[1].tk != OBJECT) {
			Pir_raise("Use A.B formed expression to get the attribute of the object");
			return 0;
		}
		if (!expr.ch[0].val) {
			unknownvar_error(expr.ch[0].name);
			return 0;
		}
		expr.trash = false;
		expr.ch[0].trash = false;
		expr.ch[1].trash = false;
		return Pir_getatttr(expr.ch[0].val,expr.ch[1].name.c_str());

	case FNCALL:
		return buildFncall(expr);

	case IMPORT:
		if (expr.ch.size() != 1) {
			importform_error();
			return 0;
		}
		if (expr.ch[0].tk == STRING) {
			interpretFile(expr.ch[0].name, true);
		}
		else if (expr.ch[0].tk == EQ) {
			if (expr.ch[0].ch.size() != 2 || expr.ch[0].ch[0].tk != OBJECT || expr.ch[0].ch[1].tk!=STRING) {
				importform_error();
				return 0;
			}
			expr.ch[0].trash = false;
			expr.ch[0].ch[0].trash = false;
			std::string lname = expr.ch[0].ch[0].name;
			if(globals.find(lname)!=globals.end()){
				redefine_error(lname); return 0;
			}
			globals[lname]=Pir_interpreter(expr.ch[0].ch[1].name);
		}
		else {
			importform_error();
			return 0;
		}
		expr.trash = false;
		return PirVoid;
	}

	args.resize(expr.ch.size());
	for (size_t i = 0; i < expr.ch.size(); i++) {
		args[i]=perform(expr.ch[i]);
		if (!args[i]) {
			Pir_raise("Null object");
			return 0;
		}
	}
	switch (expr.tk) {
	/*
	case TUPLE:
		args.release = false;
		return expr.val = (vart)args.args;*/
	case OBJECT:
		if (checkVar(expr))return 0;
		expr.trash = false;
		return expr.val;
	case NUMBER:
		return expr.val;
	case STRING:
		return expr.val = Pir_string(expr.name.c_str());
	}
	return expr.val=operate(expr.name,args[0], args.size() == 2 ? args[1] : 0);
	return PirVoid;
}

vart Interpreter::interpret(const std::string& code) {
	size_t bg = 0;
	while (bg < code.size() && IsDelimiter(code[bg]))bg++;
	if (bg == code.size())return PirVoid;

	if (st.empty())st.push({ NONE,"",0,{},false });
	size_t i = bg;
	size_t size = code.size();
	while (bg < size && i <= size) {
		_Expr& tp = st.top();
		bool cont = true;
		switch (tp.tk) {
		case STRING:
			if (i < size && code[i] == '"') {
				if (tp.comp) {
					st.push({ STRING,"",0,{},false });
					bg = i + 1;
					i = bg;
					break;
				}
				tp.comp = true;
				if (fold())return 0;
				bg = i + 1;
				i = bg;
				break;
			}
			if (!tp.comp) {
				tp.name.push_back(code[i]);
				i++;
			}
			else cont = false;
			break;
		default:
			cont = false;
			break;
		}
		if (cont)continue;

		if (code[i] == '#')break;

		if (i < size && code[i] == '.' &&
			(i == bg || IsDelimiter(code[i - 1]) || code[i - 1] == '-' || IsNum(code[i - 1]))) {
			i++;
			continue;
		}
		if (bg < i && i < size && code[i - 1] == '.' && (code[i] == 'e' || IsNum(code[i]))) {
			i++;
			continue;
		}
		if (bg < i && i < size && !IsDelimiter(code[i - 1]) && !IsDelimiter(code[i]) &&
			(lawful_ch(code[i - 1]) != (lawful_ch(code[i])))) {
			Token tk = token(bg + code.c_str(), i - bg);
			if (tk == NONE) {
				token_error(std::string(code.c_str() + bg, i - bg));
				return 0;
			}
			if (fold())return 0;
			bg = i;
			continue;
		}
		if (i < size && (code[i] == '"' || code[i] == '[' || code[i] == '(' || code[i] == '{')) {
			if (bg < i) {
				Token tk = token(bg + code.c_str(), i - bg);
				if (tk == NONE) {
					token_error(std::string(code.c_str() + bg, i - bg));
					return 0;
				}
				if (fold())return 0;
			}
			switch (code[i]) {
			case '"':
				st.push({ STRING,"",0,{},false });
				bg = i + 1;
				i = bg;
				break;
			case '[':
				st.push({ TUPLE,"",0,{},false });
				bg = i + 1;
				i = bg;
				break;
			case '(':
				st.push({ FNCALL,"",0,{},false });
				bg = i + 1;
				i = bg;
				break;
			}
			continue;
		}
		bool eoe = IsEOE(code[i]);
		if (i == size || (IsDelimiter(code[i]) || eoe)) {
			bool ch = true;
			//if (eoe)push();
			if (eoe) {
				st.push({ NONE,"",0,{},false });
				fold();
				st.pop();
			}
			if (bg < i) {
				Token tk = token(code.c_str() + bg, i - bg);
				if (tk == NONE) {
					token_error(std::string(code.c_str() + bg, i - bg));
					return 0;
				}
			}
			//for [a,] or (a,)=(a,void)
			else if ((st.top().tk == FNCALL || st.top().tk == TUPLE) && !st.top().comp) {
				if (st.top().ch.size())st.push({ OBJECT,"",PirVoid,{},true, false });
				else ch = false;
			}

			if (eoe && i < size) {
				_Expr e;
				if (ch) {
					e = st.top(); st.pop();
				}
				_Expr& t = st.top();
				switch (code[i]) {
				case ',':
					//cout << "asd " << t.tostr() <<" "<<e.tostr()<< endl;
					/*if (!t.comp && t.tk != NONE) {
						
						if (!ch)break;
						//t.ch.emplace_back(e);
						st.push(e);
						st.push({ NONE,"",0,{},false });
						fold();
						st.pop();
					}*/
					st.push(e);
					st.push({ NONE,"",0,{},false });
					fold();
					st.pop();
					break;
				case ';':
					st.push(e);
					st.push({ NONE,"",0,{},false });
					fold();
					st.pop();
					break;
				case ']':
					if (!t.comp && t.tk == TUPLE) {
						if (ch)t.ch.emplace_back(e);
						t.comp = true;
					}
					else {
						Pir_raise("InterpreterError: Cannot use ']' here");
						return 0;
					}
					break;
				case ')':
					//cout << "asd " << t.tostr() << " " << e.tostr() << endl;
					if (!t.comp && t.tk == FNCALL) {
						if (ch)t.ch.emplace_back(e);
						t.comp = true;
					}
					else {
						Pir_raise("InterpreterError: Cannot use ')' here");
						return 0;
					}
					break;
				default:
					st.push(e);
					break;
				}

			}
			if (fold())return 0;
			bg = i + 1;
			while (bg < size && IsDelimiter(code[bg]))bg++;
			i = bg;
			continue;
		}
		i++;
	}
	//cout << "top " << st.top().tostr() << endl;
	st.push({ NONE,"",0,{},false });
	fold();
	st.pop();
	trash = false;
	indent += st.size() + 1;
	if (st.top().comp) {
		if (normalize(st.top()))return 0;
		//cout <<"top "<< st.top().tostr() << endl;
		vart ret = perform(st.top());
		for (size_t i = 0; i < st.top().ch.size(); i++)
			st.top().ch[i].release();
		trash = st.top().trash;
		st.pop();
		return ret;
	}
	else return 0;
}

void Interpreter::CIL() {
	char ch;
	std::string code;
	printf(">>>");
	int indent = 0;
	while (true) {
		ch = getchar();
		if (ch == '\n' || !ch) {
			bool trash = false;
			vart obj = 0;
			try {
				obj = interpret(code);
			}
			catch (const char* msg) {
				std::cout << coloredString(msg) << "\n>>>";
				code.clear();
				while (st.size())st.pop();
				continue;
			}

			code.clear();
			if (!obj) {
				printf("null-obj\n>>>");
			}
			else {
				indent = 0;
				if (obj != PirVoid)Pir_print(obj);
				printf(">>>");
			}
			if (trash && obj) {
				Pir_release(obj);
			}
		}
		else if (ch == '\b' && code.size()) {
			code.pop_back();
			if (indent)std::cout << "\r..." << code;
			else std::cout << "\r>>>" << code;
		}
		else code.push_back(ch);
	}
}
void Interpreter::interpretFile(const std::string& file, bool raise) {
	ifstream f(file);
	if (!f.is_open()) {
		if (raise)Pir_raise("The file \"" + file + "\" does not exist");
		else cerr << coloredString("The file \"" + file + "\" does not exist\n");
		return;
	}
	while (st.size())st.pop();
	std::string line;
	size_t linenum = 1;
	while (getline(f, line)) {
		try {
			interpret(line);
		}
		catch (const char* msg) {
			std::cerr <<"line "<<linenum<<" :"<< coloredString(msg) << "\n";
			while (st.size())st.pop();
			if (raise)throw msg;
			return;
		}
		linenum++;
	}
}

Interpreter Piroof::DefaultInterpreter;

static vart inter_str(vart obj) {
	return Pir_string("<interpreter>");
}
static vart inter_new(vart obj) {
	typet type = Pir_type(obj);
	if ((vart)type == PirInterpreterType) {
		PirInterpreterObject* ret = (PirInterpreterObject*)Pir_new(PirInterpreterType);
		ret->inter = new Interpreter(*((PirInterpreterObject*)obj)->inter);
		return (vart)ret;
	}
	else if ((vart)type == PirStringType) {
		PirInterpreterObject* ret = (PirInterpreterObject*)Pir_new(PirInterpreterType);
		const char* file = ((PirStringObject*)obj)->val.c_str();
		ret->inter = new Interpreter(DefaultInterpreter);
		ret->inter->interpretFile(file,true);
		ret->attr = &ret->inter->globals;
		return(vart)ret;
	}
	convert_error(Pir_type(obj)->name, "interpreter");
	return 0;
}
static vart inter_release(vart obj) {
	PirInterpreterObject* t = (PirInterpreterObject*)obj;
	if (t->inter) {
		delete t->inter;
	}
	return 0;
}

PirTypeObject _PirInterpreterType = {
	PirTypeType,ST_CONST,0,ObjectFunc(),
	sizeof(PirInterpreterObject),"interpreter",
	inter_str,inter_new,inter_release
};

vart Piroof::PirInterpreterType = (vart)&_PirInterpreterType;

vart Piroof::Pir_interpreter(const std::string& file) {
	PirInterpreterObject* ret = (PirInterpreterObject*)Pir_new(PirInterpreterType);
	ret->inter = new Interpreter(DefaultInterpreter);
	ret->inter->interpretFile(file,true);
	ret->attr = &ret->inter->globals;
	return(vart)ret;
}
