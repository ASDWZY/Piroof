#pragma once

#include "common.h"
#include "number.h"

namespace Piroof {
	struct PirObject {
		void* type;
		uint8 state;
		std::unordered_map<std::string,PirObject*>* attr=0;
	};
	typedef PirObject* vart;
	typedef std::unordered_map<std::string, PirObject*>* attrmap;
	typedef vart(*objfunc1)(vart);
	typedef vart(*objfunc2)(vart,vart);
	typedef Dict<vart> Arguments;
	typedef vart(*objcallback)(vart, const Arguments&);

#define ST_CONST 1
#define ST_ARGC 2
#define ST_DEL 4
#define ST_SHARED 8
#define PirVarHead void* base;uint8 state=ST_CONST;attrmap attr
	
	struct ObjectFunc {
		objcallback f=0;
		Dict<vart> p; bool nocheck = false;
		ObjectFunc() {}
		ObjectFunc(objcallback _f) {
			f = _f; nocheck = true;
		}
		ObjectFunc(objcallback _f,const std::vector<Dict<vart>::Node>& pattern) {
			f = _f;
			p.init(pattern);
		}
		std::string check(const Arguments& args)const {
			if (nocheck)return "";
			for (const auto& i : p) {
				if (!i.x && !args.find(i.str))
					return "Missing a required argument \""+i.str+"\"";
			}
			for (const auto& i : args) {
				if (!p.find(i.str))
					return "Unexpected argment \"" + i.str + "\"";
			}
			return "";
		}
		vart operator()(vart,const Arguments& args)const;
		operator bool()const {
			return f;
		}
	};

	struct PirTypeObject {
		PirVarHead; ObjectFunc _call;
		size_t size;
		String name;
		objfunc1 _str, _new, _release;
	};

	template<typename T>
	struct PirValObject {
		PirVarHead;
		T val;
	};
	typedef PirValObject<String> PirStringObject;
	

	struct PirExceptionObject {
		PirVarHead;
		String msg;
	};

	struct PirVoidObject {
		PirVarHead;
	};
	extern vart PirTypeType, PirStringType, PirExceptionType, PirVoidType, PirVoid;

	typedef PirTypeObject* typet;
	vart Pir_new(vart type,uint8 state=ST_CONST);
	void Pir_forceRelease(vart obj);
	void Pir_release(vart obj);
	size_t Pir_sizeof(vart obj);
	typet Pir_type(vart obj);
	void Pir_exit(int code);

	vart Pir_exception(vart type,const String& msg);
	void Pir_throw(vart exc);
	void Pir_error(const String& msg);
	void Pir_raise(const String& msg);

	vart Pir_tostr(vart);
	vart Pir_string(const String&);
	void Pir_print(vart);

	vart Pir_call(vart, const Arguments&);
	vart Pir_copyvar(vart obj);
	vart Pir_copybytes(vart,uint8 state);
	vart Pir_getatttr(vart, const char* a);

	struct PirBFObject {
		PirVarHead;
		ObjectFunc f;
	};
	extern vart PirBFType;
	extern vart PirPrint, PirExit, PirIsconst;

	typedef PirValObject<LogicState> PirLogicObject;
	typedef PirValObject<RationalNumber> PirNumberObject;
	typedef PirValObject<double> PirFloatObject;
	typedef PirValObject<Integer> PirIntObject;
	extern vart PirLogicType, PirNumberType, PirFloatType, PirIntType;
	extern vart PirTrue, PirFalse, PirNotsure, PirContra, PirInf, PirInd;

	vart Pir_logic(LogicState ls);
	vart Pir_number(const RationalNumber& n);
	vart Pir_float(double x);
	vart Pir_int(const Integer& x);
}