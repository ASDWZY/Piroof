#pragma once
#include "../number.h"
#include "../interpreter.h"

namespace Piroof {
	struct Expr {
		static MemoryPoolForList<ListNode<Expr>> Pir_exprmempool;
		typedef List<Expr, Pir_exprmempool,uint64> cht;
		typedef _String<uint64> strt;
		//std::vector<Expr> ch;
		cht ch;
		void* val = 0;
		int8 tk=NONE,laz=0;
		RationalNumber c = 1;

		Expr() {}
		bool empty()const {
			return tk == NONE;
		}
		Expr(const RationalNumber& num) {
			tk = NUMBER;
			c = num;
		}
		void release() {
			//std::cout << "releasing " << serialize(DefaultInterpreter) << std::endl;
			for (Expr& i : ch)i.release();
			if (val)Pir_release((vart)val);
		}
		Expr(int8 op, const Expr& a) {
			tk = op;
			ch.emplace_back(a);
			normalize(false);
		}
		Expr(int8 op, const Expr& a, const Expr& b) {
			tk = op;
			ch.emplace_back(a);
			ch.emplace_back(b);
			normalize(false);
		}
		Expr(const Expr& e) {
			*this = e;
		}
		bool isNum()const {
			return tk == NUMBER;
		}

		void operator=(const Expr& e) {
			ch = e.ch; 
			val = e.val;
			tk = e.tk;
			c = e.c; 

			laz = e.laz;
		}
		bool operator==(const Expr& e)const {
			if (val!=e.val||tk != e.tk || bool(c != e.c) || ch.size() != e.ch.size())
				return false;

			auto i = ch.begin(),
				j = e.ch.begin();
			while (i != ch.end()) {
				if (*i != *j)
					return false;
				++i; ++j;
			}
			//for (size_t i = 0; i < ch.size(); i++)if (ch[i] != e.ch[i])return false;
			return true;
		}
		bool operator!=(const Expr& e)const {
			return !(*this == e);
		}
		int priority(const Interpreter& inter)const;
	    strt serialize(const Interpreter& inter)const;

		friend Expr operator+(const Expr& b, const Expr& a) {
			if (b.tk==ADD) {
				Expr ret = b;
				ret.ch.emplace_back(a);
				return ret;
			}

			return Expr(ADD, b, a);
		}

		friend Expr operator*(const Expr& b, const Expr& a) {
			if (a.tk==NUMBER)
				return Expr(MUL, b, a);
			if (b.tk==NUMBER) {
				Expr x = a;
				x.c = b.c;
				return Expr(ADD, x);
			}
			if (b.tk==MUL) {
				Expr ret = b;
				ret.ch.emplace_back(a);
				return ret;
			}
			return Expr(MUL, b, a);
		}

		Expr operator-()const {
			if (tk==NUMBER)
				return -c;
			return Expr(ADD,Expr(-1), *this);
		}

		friend Expr operator-(const Expr& a,const Expr& b) {
			if (a.tk == ADD) {
				Expr ret = a;
				ret.ch.emplace_back(a);
				ret.ch.back().c = -1;
				return ret;
			}
			Expr ret(ADD, a, b);
			ret.ch.back().c = -1;
			return ret;
		}

		friend Expr operator/(const Expr& a,const Expr& b) {
			return Expr(DIV, a, b);
		}

		friend Expr operator^(const Expr& a,const Expr& b) {
			//delete it
			if (b.tk==NUMBER && a.tk!=NUMBER) {
				Expr x = a;
				x.c = b.c;
				return Expr(MUL, x);
			}
			return Expr(POW, a, b);
		}

		void normalize(bool tree) {
			//std::cout << "normalize " << serialize(DefaultInterpreter) << std::endl;
			if (tree)
				for (Expr& i : ch) {
					i.normalize(true);
				}

			if (tk!=ADD&&tk!=MUL)
				return;
			if (ch.size() == 1) {
				if (bool(ch.front().c == 1)) {
					RationalNumber x = c;
					Expr t = ch.front();
					*this = t;
					c = x;
				}
				if (ch.front().tk == NUMBER) {
					Expr t = ch.front();
					*this = t;
				}
			}
			if (ch.size() && bool(ch.front().c == 1) && ch.front().tk == tk) {
				//std::vector<Expr> nch = ch.front().ch;
				cht nch = ch.front().ch;
				auto p = ch.begin();
				++p;
				nch.insert(nch.end(),p,ch.end());
				
				ch = nch;
			}

		}
		void replace(const Interpreter& inter, const std::vector<Expr>& args);
		void calc_laz() {
			switch (tk) {
			case NUMBER:
				laz = 1;
				return;
			case OBJECT:
				laz = 1;
				return;
			case CONSTANT:
				laz = 1;
				return;
			case VAR:
				laz = 2;
				return;
			case SYMBOL:
				laz = 2;
				return;
			}
			laz = 0;
			for (Expr& i : ch) {
				i.calc_laz();
				laz |= (i.laz - 1);
			}
			laz++;
		}
		void removeCoeff() {
			
			if (tk == ADD) {
				for (Expr& i : ch) {
					if (i.c == 1)continue;
					Expr t = i; t.c = 1;
					i = Expr(MUL, i.c, t);
				}
			}
			else if (tk == MUL) {
				for (Expr& i : ch) {
					if (i.c == 1)continue;
					Expr t = i; t.c = 1;
					i = Expr(POW, t, i.c);
				}
			}
			//for (Expr& i : ch)i.removeCoeff();
		}
	};

	struct PirExprObject {
		PirVarHead;
		Expr* expr;
		int8 pos=-1,numargs=0;
		Expr::strt s;
	};
	extern vart PirExprType;
	vart Pir_numexpr(const RationalNumber& n);
	vart Pir_symexpr(const std::string& s, Token tk, int8 pos=-1);
	vart Pir_expr(const Expr& expr);
	vart Pir_expr(vart obj);

	extern vart PirExpand, PirIsconstexpr, PirSortexpr;


}