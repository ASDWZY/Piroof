#include "Piroof.h"
using namespace Piroof;
using namespace std;

MemoryPoolForList<ListNode<Expr>> Piroof::Expr::Pir_exprmempool;

int Expr::priority(const Interpreter& inter)const {
	if (ch.size() == 1) {
		switch (tk) {
		case ADD:
			return inter.operators[MUL].p;
		case MUL:
			return inter.operators[POW].p;
		}
	}
	switch (tk) {
	case NUMBER:
		return 100;
	case SYMBOL:
		return 100;
	case VAR:
		return 100;
	case CONSTANT:
		return 100;
	}
	return inter.operators[tk].p;
}

Expr::strt Expr::serialize(const Interpreter& inter)const{

	strt ret;
	bool first = true;
	switch (tk) {
	case VAR:
		return ((PirExprObject*)val)->s;
	case SYMBOL:
		return ((PirExprObject*)val)->s;
	case CONSTANT:
		return ((PirExprObject*)val)->s;
	case NUMBER:
		return c.ToString<strt>();
	case OBJECT: {
		vart str=Pir_tostr((vart)val);
		ret = ((PirStringObject*)str)->val.c_str();
		Pir_release(str);
		return ret;
	}
	case FUNCTION:
		ret = ((PirExprObject*)val)->s + "(";
		for (const Expr& i:ch)
			ret += i.serialize(inter) + ",";
		if (ch.size())ret.back() = ')';
		else ret.push_back(')');
		return ret;
	case ADD:
		if (!ch.size())return "sum()";
		for (const Expr& i : ch) {
			if (i.tk == NUMBER) {
				if (!first)ret.push_back('+');
				ret += i.c.ToString<strt>();
				first = false;
				continue;
			}
			if (!first&&bool(i.c.P()>=0))
				ret.push_back('+');
			if (i.c == -1)
				ret.push_back('-');
			else if (i.c != 1) {
				ret += i.c.ToString<strt>();
			}
			if ((first && bool(i.c != 1) && i.priority(inter) < inter.operators[MUL].p) ||
				(first && i.priority(inter) < inter.operators[ADD].p))
				ret += "(" + i.serialize(inter) + ")";
			else ret += i.serialize(inter);
			first = false;
		}
		return ret;
	case MUL:
		if (!ch.size())return "multiply()";
		break;
	}
	std::string opname = inter.getOpname(tk);
	if (!ch.size())return opname + "()";
	int p = priority(inter);
	if (ch.front().priority(inter) < p)ret += "(" + ch.front().serialize(inter) + ")";
	else ret += ch.front().serialize(inter);
	if (tk == MUL) {
		if (ch.front().tk != NUMBER&&bool(ch.front().c != 1)) {
			const RationalNumber& num = ch.front().c;
			if (num.IsFloat())ret += "^(" + num.ToString<strt>() + ")";
			else ret += "^" + num.ToString<strt>();
		}
	}
	else if (ch.size() == 1)return inter.operators[tk].m & OP_PRE ? opname + ret : ret + opname;

	auto i = ch.begin(); ++i;
	while(i!=ch.end()) {
		Expr& tmp = *i;
		ret += opname;
		if (tmp.priority(inter) <= p)
			ret += "(" + tmp.serialize(inter) + ")";
		else ret += tmp.serialize(inter);

		if (tk == MUL&&tmp.tk!=NUMBER&& bool(tmp.c != 1)) {
			const RationalNumber& num = tmp.c;
			if (num.IsFloat())ret += "^(" + num.ToString<strt>() + ")";
			else ret += "^" + num.ToString<strt>();
		}
		++i;
	}
	return ret;
}
void Expr::replace(const Interpreter& inter, const std::vector<Expr>& args) {
	if(tk==VAR){
		*this = args[((PirExprObject*)val)->pos];
		return;
	}
	if (tk == ADD) {
		for (Expr& i : ch) {
			if (i.tk == VAR && bool(i.c != 1))
				i = Expr(MUL,i.c,args[((PirExprObject*)i.val)->pos]);
			else
				i.replace(inter, args);
		}
	}
	if (tk == MUL) {
		for (Expr& i : ch) {
			if (i.tk == VAR && bool(i.c != 1))
				i = Expr(POW,args[((PirExprObject*)i.val)->pos],i.c);
			else
				i.replace(inter, args);
		}
	}
	for (Expr& i : ch)i.replace(inter, args);
}


static vart expr_str(vart obj) {
	PirExprObject* t = (PirExprObject*)obj;
	if (t->pos == -2) {
		return Pir_string("<function-expression>");
	}
	return Pir_string(t->expr->serialize(DefaultInterpreter).c_str());
}
static void _expr_new(vart obj, Expr& e) {
	typet type = Pir_type(obj);
	if ((vart)type == PirExprType) {
		e = *((PirExprObject*)obj)->expr;
		return;
	}
	else if ((vart)type == PirNumberType) {
		e.val = 0;
		e.tk = NUMBER;
		e.c = ((PirNumberObject*)obj)->val;
		return;
	}
	e.val = Pir_copyvar(obj);
	e.tk = OBJECT;
}
static vart expr_new(vart obj) {
	typet type = Pir_type(obj);
	if ((vart)type == PirExprType) {
		Expr& e = *((PirExprObject*)obj)->expr;
		//if (e.tk == SYMBOL || e.tk==CONSTANT)return obj;
	}
	PirExprObject* ret = (PirExprObject*)Pir_new(PirExprType);
	ret->expr = new Expr();
	_expr_new(obj, *ret->expr);
	return (vart)ret;
}
static vart expr_release(vart obj) {
	PirExprObject* e = (PirExprObject*)obj;
	if (e->expr) {
		if (e->expr->tk == SYMBOL|| e->expr->tk==CONSTANT)return (vart)1;
		e->expr->release();
		delete e->expr;
		e->expr = 0;
	}
	return 0;
}

static vart expr_call(vart obj,const Arguments& args) {
	PirExprObject* e = (PirExprObject*)obj;
	if (e->pos != -2) {
		Pir_raise("This expr cannot be called");
		return 0;
	}
	if (args.size() != size_t(e->numargs)) {
		Pir_raise("Number of arguments should be " + toString(int(e->numargs)) +
			" but got " + toString(args.size()));
	}
	std::vector<Expr> ea(args.size());
	for (size_t i = 0; i < ea.size(); i++)
		_expr_new(args[i], ea[i]);
	Expr ex = *e->expr;
	ex.replace(DefaultInterpreter,ea);
	return Pir_expr(ex);
}
PirTypeObject _PirExprType{
	PirTypeType,ST_CONST,0,
	ObjectFunc(expr_call),sizeof(PirExprObject),
	"expr",expr_str,expr_new,expr_release
};
vart Piroof::PirExprType = (vart)&_PirExprType;

vart Piroof::Pir_numexpr(const RationalNumber& n) {
	PirExprObject* e = (PirExprObject*)Pir_new(PirExprType);
	e->expr = new Expr();
	e->expr->val=0;
	e->expr->tk = NUMBER;
	e->expr->c = n;
	return (vart)e;
}

vart Piroof::Pir_symexpr(const std::string& name, Token tk, int8 pos) {
	PirExprObject* e = (PirExprObject*)Pir_new(PirExprType,ST_CONST|ST_SHARED);
	e->expr = new Expr();
	e->expr->val = (vart)e;
	e->expr->tk = tk;
	e->s = name;
	e->pos = pos;
	return (vart)e;
}

vart Piroof::Pir_expr(const Expr& expr) {
	PirExprObject* e = (PirExprObject*)Pir_new(PirExprType);
	e->expr = new Expr(expr);
	return (vart)e;
}

vart Piroof::Pir_expr(vart obj) {
	return expr_new(obj);
}

static void expand(Expr& e);

inline Expr::cht exMul(const Expr& e, bool rc = false) {
	if (e.tk==MUL)return e.ch;
	if (e.tk==ADD && e.ch.size() == 1) {
		return exMul(e.ch.front());
	}
	Expr::cht ret;
	ret.emplace_back(e);
	if (rc)ret.front().c = 1;
	return ret;
}

inline Expr mulExpr(Expr a, Expr b, bool adda,bool addb) {
	RationalNumber c = 1;
	if (adda && !a.isNum() && bool(a.c != 1)) {
		c *= a.c; a.c = 1;
	}
	if (addb&&!b.isNum() && bool(b.c != 1)) {
		c *= b.c; b.c = 1;
	}

	if (a.isNum()) {
		if (b.isNum())
			return a.c * b.c;
		Expr ret = b;
		ret.c = a.c;
		return ret;
	}
	if (b.isNum()) {
		Expr ret = a;
		ret.c = b.c;
		return ret;
	}

	Expr ret;
	ret.tk=MUL;
	ret.c = c;
	ret.ch = exMul(a);
	Expr::cht t = exMul(b);
	ret.ch.insert(
		ret.ch.end(),
		t.begin(), t.end()
	);
	return ret;
}

static Expr::cht expandAdd(Expr e) {
	Expr::cht ret;
	e.normalize(false);

	if (e.tk==ADD) {
		for (Expr& i : e.ch) {
			if (i.tk==ADD) {
				i.ch = expandAdd(i);
				for (Expr& j : i.ch)
					j.c *= i.c;
				ret.insert(ret.end(),i.ch.begin(),i.ch.end());
			}
			else
				ret.emplace_back(i);
		}
		return ret;
	}
	else if (e.tk==MUL && e.ch.size() > 1) {
		Expr& a = e.ch.front();
		Expr b = e;
		b.ch.pop_front();
		b.ch = expandAdd(b);
		if (a.tk==ADD) {
			for (Expr& i : a.ch) {
				for (Expr& j : b.ch) {
					ret.emplace_back(mulExpr(i, j, true, true)); 
					//cout << "back " << ret.back().c.ToString<String>() << endl;
					ret.back().normalize(false);
				}
			}
		}
		else {
			for (Expr& i : b.ch) {
				ret.emplace_back(mulExpr(a, i, false, true)); 
				ret.back().normalize(false);
			}
		}

	}
	else ret.emplace_back(e);
	return ret;
}

static bool cmpAddExpr(const Expr& a, const Expr& b) {
	//cout << "cmp " << a.serialize(DefaultInterpreter) << " " << int(a.laz) << endl;
	//cout << "cmp " << b.serialize(DefaultInterpreter) << " " << int(b.laz) << endl;
	//return a.serialize(DefaultInterpreter) > b.serialize(DefaultInterpreter);
	int da = int(a.laz==2), db = int(b.laz==2);
	//a.print();b.print();
	//cout<<"a "<<da<<" "<<db<<endl;
	if (!da || !db) {
		if (da == db)
			return a.serialize(DefaultInterpreter) < b.serialize(DefaultInterpreter);
		return da > db;
	}
	Expr::cht ma = exMul(a, true),mb = exMul(b, true);
	auto i = ma.begin(), j = mb.begin();
	int na = ma.size(), nb = mb.size();
	while (na && (*i).laz==1)++i,na--;
	while (nb && (*j).laz==1)++j, nb--;
	while (i != ma.end() && j != mb.end()) {
		Expr& ti = *i; Expr& tj = *j;
		Expr::strt sa = ti.serialize(DefaultInterpreter);
		Expr::strt sb = tj.serialize(DefaultInterpreter);
		if (sa != sb)return sa < sb;
		if (ti.c != tj.c)return ti.c > tj.c;
		++i; ++j;
	}
	return ma.size()>mb.size();
}

static bool cmpMulExpr(const Expr& a, const Expr& b) {
	if (a.laz != b.laz)
		return a.laz < b.laz;
	int na = int(a.isNum());
	int nb = int(b.isNum());
	if (na || nb)return na < nb;
	return a.serialize(DefaultInterpreter) < b.serialize(DefaultInterpreter);
}

inline bool sameTerm(Expr a,const Expr& b) {
	if (a.isNum() && b.isNum())
		return true;
	a.c = b.c;
	return a == b;
}

inline Expr mergeTerms(Expr& e) {
	Expr ret = e;
	ret.ch.resize(0);
	auto i = e.ch.begin(), j = i;
	++j;
	while (i != e.ch.end()) {
		Expr& a = *i;
		while (j != e.ch.end() && sameTerm(a, *j)) {
			a.c += (*j).c;
			++i; ++j;
		}
		if (a.c!=0)ret.ch.emplace_back(a);
		//cout << "move " << (*i).serialize(DefaultInterpreter) << " " << (*j).serialize(DefaultInterpreter) << endl;
		++i; ++j;
	}
	if (!ret.ch.size()) {
		if (ret.tk==ADD)return RationalNumber(0);
		if (ret.tk==MUL)return RationalNumber(1);
	}
	return ret;
}
typedef bool(*cmpt)(const Expr&, const Expr&);

inline void _sortExpr(Expr& e) {
	//cout<<"sort ";e.print();

	for (Expr& i : e.ch)
		_sortExpr(i);
	if (e.tk==ADD) {
		sortList(e.ch, cmpAddExpr);
		e = mergeTerms(e);
	}
	else if (e.tk==MUL) {
		sortList(e.ch, cmpMulExpr);
		e = mergeTerms(e);
	}
}

inline void sortExpr(Expr& e) {
	//cout << "sorting " << e.serialize(DefaultInterpreter) << endl;
	e.normalize(true);
	if(!e.laz)e.calc_laz();
	_sortExpr(e);
	/*t.normalize(true);
	while (t != e) {
		//cout << "sorting " << t.serialize(DefaultInterpreter) << endl;
		e = t;
		e.calc_laz();
		Expr t = _sortExpr(e);
		t.normalize(true);
	}*/
}

inline Expr::cht mulExpr(const Expr::cht& a, const Expr::cht& b) {
	Expr::cht ret;
	if (!a.size())return b;
	if (!b.size())return a;
	for (const Expr& i : a) {
		for (const Expr& j : b) {
			//ret.emplace_back(i*j);
			ret.emplace_back(mulExpr(i,j,true,true));
			sortExpr(ret.back());
		}
	}
	return ret;
}

static void expand(Expr& e) {
	for (Expr& i : e.ch)
		expand(i);
	if (e.tk == MUL) {
		for (Expr& a : e.ch) {
			//pow expanding
			if (bool(a.c > 1) && bool(a.c < 8) && !a.c.IsFloat() &&
				a.priority(DefaultInterpreter) != 100) {
				int64 p = a.c.P();
				Expr::cht base;
				if (a.tk == ADD)base = a.ch;
				else base.emplace_back(a);
				a.ch.clear();
				a.tk = ADD;
				a.c = 1;
				while (p) {
					if (p & 1)a.ch = mulExpr(a.ch, base);
					base = mulExpr(base, base);
					p >>= 1;
				}
				//if (!a.ch.size())a.ch.emplace_back(RationalNumber(1));
				//cout << "a " << a.serialize(DefaultInterpreter) << endl;
			}
		}
		e.ch = expandAdd(e);
		e.tk = ADD;
		e.c = 1;
	}
	else if (e.tk == ADD) {
		e.ch = expandAdd(e);
		for (Expr& i : e.ch) {
			if (bool(i.c == 1) || i.tk != ADD)
				continue;
			for (Expr& j : i.ch)
				j.c *= i.c;
			i.c = 1;
		}
	}
	if (!e.laz)e.calc_laz();
	if (e.tk == ADD) {
		sortList(e.ch, cmpAddExpr);
		e = mergeTerms(e);
	}
	else if (e.tk == MUL) {
		sortList(e.ch, cmpMulExpr);
		e = mergeTerms(e);
	}
	e.normalize(false);
}

static vart Pir_expand_f(vart obj, const Arguments& args) {
	typet type = Pir_type(args[0]);
	Expr ret;
	if ((vart)type != PirExprType) {
		PirExprObject* e = (PirExprObject*)Pir_expr(args[0]);
		ret = *e->expr;
		Pir_release((vart)e);
	}
	else {
		PirExprObject* e = (PirExprObject*)args[0];
		ret = *e->expr;
	}
	expand(ret);
	sortExpr(ret);
	return Pir_expr(ret);
}
PirBFObject Pir_expand_BF = {
	PirBFType,ST_CONST | ST_ARGC,0,
	ObjectFunc(Pir_expand_f,{{"expression",0}})
};
vart Piroof::PirExpand = (vart)&Pir_expand_BF;

static vart Pir_isconstexpr_f(vart obj, const Arguments& args) {
	typet type = Pir_type(args[0]);
	if ((vart)type != PirExprType) {
		return Pir_logic(notsure);
	}
	PirExprObject* e = (PirExprObject*)args[0];
	if (e->expr->laz == 0)e->expr->calc_laz();
	return Pir_logic(e->expr->laz==1);
}
PirBFObject Pir_isconstexpr_BF = {
	PirBFType,ST_CONST | ST_ARGC,0,
	ObjectFunc(Pir_isconstexpr_f,{{"expression",0}})
};
vart Piroof::PirIsconstexpr = (vart)&Pir_isconstexpr_BF;

static vart Pir_sortexpr_f(vart obj, const Arguments& args) {
	typet type = Pir_type(args[0]);
	Expr ret;
	if ((vart)type != PirExprType) {
		PirExprObject* e = (PirExprObject*)Pir_expr(args[0]);
		ret = *e->expr;
		Pir_release((vart)e);
	}
	else {
		PirExprObject* e = (PirExprObject*)args[0];
		ret = *e->expr;
	}
	sortExpr(ret);
	return Pir_expr(ret);
}
PirBFObject Pir_sortexpr_BF = {
	PirBFType,ST_CONST | ST_ARGC,0,
	ObjectFunc(Pir_sortexpr_f,{{"expression",0}})
};
vart Piroof::PirSortexpr = (vart)&Pir_sortexpr_BF;
