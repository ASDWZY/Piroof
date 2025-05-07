#pragma once

#include "objects.h"

#include <stack>

#define OP_NONE 0
#define OP_PRE 1
#define OP_MID 2
#define OP_POST 4

namespace Piroof {

	typedef enum{
		NONE,
		OBJECT,

		STRING,
		FNCALL,
		TUPLE,
		DICT,

		IF,
		FOR,
		WHILE,
		DELETE,

		ADD,
		SUB,
		MUL,
		DIV,
		POW,

		EQ,
		GETATTR,
		//CMP_EQ,
		LT,
		GT,
		LE,
		GE,

		NOT,
		OR,
		AND,
		IMPLIES,
		EQUIVALENT,

		SYMBOL,
		CONSTANT,
		NUMBER,
		VAR,
		ASSIGN,
		FUNCTION,

		LET,
		SUPPOSE,
		IMPORT,

		MAX_TOKEN,
	}Token;


	class Interpreter {
	public:
		struct _Expr {
			Token tk;
			std::string name;
			vart val = 0;
			std::vector<_Expr> ch;
			bool comp = false;
			bool trash = true;
			void release();
			std::string tostr() {
				std::string ret = name +toString(int(tk))+ "(";
				for (size_t i = 0; i < ch.size(); i++)
					ret += ch[i].tostr() + ",";
				if(comp)
				if (ch.size())ret.back() = ')';
				else ret.push_back(')');
				return ret;
			}
		};
		struct Operator {
			int p = 0;
			char m = 0;
			bool post = false;
			std::string name;
		};
		
		typedef std::map<std::string,vart> expr_map;
		typedef vart(*opcallback)(vart, vart);
		std::unordered_map<std::string, vart> globals;
		std::unordered_map<std::string, Token> tkmp;
		std::unordered_map<std::string, opcallback> opcallbacks;
		//std::map<std::string, vart> globals;
		std::vector<Operator> operators;
		std::stack<_Expr> st;

		size_t indent = 0;
		bool trash = false;
		std::string name="cil interpreter";

		Interpreter();
		Interpreter(const std::string& _name) {
			name = _name;
		}
		~Interpreter();
		void release();
		void clear();

		std::string getOpname(int tk)const;
		vart operate(const String& op, vart a, vart b);
		
		vart interpret(const std::string&);
		void CIL();
		void interpretFile(const std::string& file,bool raise=false);
	private:
		void initGlobals();
		void initOperators();
		void initTkmp();
		void loadOperatorCallbacks();

		bool findvar(const std::string& name) {
			return globals.find(name) != globals.end();
		}
		Token token(const char* bg, size_t len);
		int fold();
		int push();
		int buildExpr(_Expr& expr, expr_map&);
		vart createVar(_Expr& expr,Token tk, bool assign,const std::string& tkname);
		vart perform(_Expr& expr,Token tk=NONE);
		int checkVar(const _Expr& expr);
		int normalize(_Expr& expr);
		vart buildFncall(_Expr&f);
	};

	extern Interpreter DefaultInterpreter;

	struct PirInterpreterObject {
		PirVarHead;
		Interpreter* inter=0;
	};
	extern vart PirInterpreterType,PirInterpreter;
	vart Pir_interpreter(const std::string& file);
}