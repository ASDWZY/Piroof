#pragma once

#include "common.h"

namespace Piroof {
	template<typename T>
	T abs(const T& x) {
		if (x >= T(0))return x;
		else return -x;
	}

	template<typename T>
	int sgn(const T& x) {
		if (x > T(0))return 1;
		else if (x < T(0))return -1;
		else return 0;
	}

	template<typename T>
	T gcd(const T& x, const T& y) {
		if (y)return gcd(y, x % y);
		else return x;
	}

	template<typename T>
	T lcm(const T& x, const T& y) {
		return x / gcd(x, y) * y;
	}

	template<typename T1, typename T2>
	T1 fastpow(T1 base, T2 pow) {
		T1 ans = 1;
		while (pow) {
			if (pow & 1)ans *= base;
			pow >>= 1;
			base *= base;
		}
		return ans;
	}

	inline bool isNumber(const char* str) {
		uint64 size = strlen(str);
		if (!size)return false;

		bool P = false, F = false;
		uint64 epos = 0;
		for (uint64 i = 0; i < size; i++) {
			switch (str[i]) {
			case '.':
				if (size==1||P)return false;
				P = true;
				break;
			case 'e':
				if (size < 3 || epos || i == 0 || i == size - 1)return false;
				epos = i + 1;
				break;
			case '-':
				if (size < 2 || i == 0 || str[i - 1] != 'e')return false;
				break;
			default:
				if (str[i] < '0' || str[i]>'9')return false;
			}
		}
		return true;
	}

	inline int ParseNumber(std::string& str, int64& e, bool& neg) {
		e = 0; neg = false;
		if (!str.size())
			return -1;
		if (str[0] == '-') {
			neg = true;
			if (str.size() == 1)return -1;
			str = str.substr(1, str.size() - 1);
		}
		if (str == "inf")return 2;

		bool P = false, F = false, isFloat = false;
		uint64 epos = 0;
		for (uint64 idx = 0; idx < str.size(); idx++) {
			switch (str[idx]) {
			case '.':
				isFloat = true;
				if (P)return -1;
				P = true;
				break;
			case 'f':
				isFloat = true;
				if (F || idx < str.size() - 1 || idx == 0)return -1;
				F = true;
				str.pop_back();
				break;
			case 'e':
				isFloat = true;
				if (epos || idx == 0 || idx == str.size() - 1)return -1;
				epos = idx + 1;
				break;
			case '-':
				if (idx == 0 || str[idx - 1] != 'e')return -1;
				break;
			default:
				if (str[idx] < '0' || str[idx]>'9')return -1;
			}
		}

		try {
			if (epos) {
				e = stoll(str.substr(epos, str.size() - epos));
				str.resize(epos - 1);
			}
		}
		catch (...) {
			e = 0;
			return -1;
		}
		return isFloat;
	}

	class LogicState {
	public:
		LogicState() {}
		LogicState(bool x) {
			s = x ? 2 : 1;
		}
		bool isTrue()const {
			return s == 2;
		}
		bool isFalse()const {
			return s == 1;
		}
		bool isNotsure()const {
			return s == 3;
		}
		bool isContra()const {
			return s == 0;
		}
		static LogicState Notsure() {
			LogicState ret; ret.s = 3;
			return ret;
		}
		static LogicState Contra() {
			return LogicState();
		}
		friend LogicState operator|(LogicState a,LogicState b){
			LogicState ret;
			ret.s=a.s | b.s;
			return ret;
		}
		friend LogicState operator&(LogicState a,LogicState b) {
			LogicState ret;
			ret.s = a.s & b.s;
			return ret;
		}
		operator bool()const {
			return isTrue();
		}
		LogicState operator!()const {
			LogicState ret;
			ret.s = s >> 1 | (s&1)<<1;
			return ret;
		}
		friend LogicState operator||(LogicState a, LogicState b) {
			if (a.isTrue() || b.isTrue())return true;
			if (a.isContra() || b.isContra())return Contra();
			if (a.isNotsure() || b.isNotsure())return Notsure();
			return false;
		}
		friend LogicState operator&&(LogicState a, LogicState b) {
			if (a.isFalse() || b.isFalse())return false;
			if (a.isContra() || b.isContra())return Contra();
			if (a.isNotsure() || b.isNotsure())return Notsure();
			return true;
		}
		friend LogicState operator>>(LogicState a, LogicState b) {
			if (b)return true;
			if (a.isTrue())return b;
			if (a.isFalse())return false;
			if (a.isNotsure())return LogicState(false) | b;
			return LogicState(false) & b;
		}
		friend LogicState operator==(LogicState a, LogicState b) {
			if (a.isContra() || b.isContra())return Contra();
			if (a.isNotsure() || b.isNotsure())return Notsure();
			return a.s==b.s;
		}
		String toString()const {
			switch (s) {
			case 0:
				return "contra";
			case 1:
				return "false";
			case 2:
				return "true";
			case 3:
				return "notsure";
			}
		}
	private:
		uint8 s=0;
	};
	extern LogicState notsure, contra;

	int ParseNumber(std::string& str, int64& e, bool& neg);

	class Number {
	public:
		static int64 eMin, eMax;
		static void eRange(int64 _eMin, int64 _eMax);

		Number();
		Number(const char*);
		Number(const std::string&);
		Number(int8);
		Number(uint8);
		Number(int16);
		Number(uint16);
		Number(int32);
		Number(uint32);
		Number(int64);
		Number(uint64);
		Number(float32);
		Number(float64);
		void Init(const std::string&, bool E = true);

		std::string Numbers()const;
		std::string ToString()const;
		friend std::ostream& operator<<(std::ostream&, const Number&);

		template<typename p_t, typename q_t>
		void ToFraction(p_t& P, q_t& Q)const {
			if (!mStr.size()) {
				P = p_t(0);
				Q = p_t(1);
				return;
			}
			int64 m = MinorSize();
			P = stringTo<p_t>(std::string(mStr.c_str() + mBegin));
			if (m <= 0) {
				Q = q_t(1);
				P *= fastpow(p_t(10), p_t(-m));
			}
			else {
				Q = fastpow(q_t(10), q_t(m));
			}
			if (mNegative)P = -P;
		}

		LogicState IsInteger()const;
		LogicState IsFloat()const;

		void Normalize(bool E = true);
		void ScientificNotation();
		void eNo();
		int64 e()const;
		int64 MinorSize()const;

		Number operator>>(int64)const;
		Number operator<<(int64)const;
		void operator>>=(int64);
		void operator<<=(int64);

		friend LogicState operator==(const Number&, const Number&);
		friend LogicState operator<(const Number&, const Number&);
		friend LogicState operator>(const Number&, const Number&);
		friend LogicState operator<=(const Number&, const Number&);
		friend LogicState operator>=(const Number&, const Number&);

		friend Number operator+(const Number&, const Number&);
		Number operator-()const;
		friend Number operator-(const Number&, const Number&);
		friend Number operator*(const Number&, const Number&);
		template<typename T>
		friend T operator/(const Number& x, const Number& y) {
			return T(x) / T(y);
		}

		void operator+=(const Number&);
		void operator-=(const Number&);
		void operator*=(const Number&);

	private:
		bool mNegative;
		std::string mStr;
		uint64 mBegin;
		int64 mE, mPos;

		int64 eNotation();
		void eMove(int64);
		int64 Offset(const Number&)const;
	};

	class Integer {
	public:
		typedef uint8 _size_t;
		Integer();
		Integer(int8);
		Integer(uint8);
		Integer(int16);
		Integer(uint16);
		Integer(int32);
		Integer(uint32);
		Integer(int64);
		Integer(uint64);
		Integer(const char*);
		Integer(const std::string&);
		Integer(const Number& n);

		std::string ToString()const;
		std::string Bytes()const;
		_size_t Size()const;
		uint8 Sign()const;
		void Normalize()const;
		uint8 operator[](_size_t)const;
		friend std::ostream& operator<<(std::ostream&, const Integer&);

		friend Integer operator<<(const Integer&, _size_t);
		friend Integer operator>>(const Integer&, _size_t);
		void operator<<=(_size_t);
		void operator>>=(_size_t);

		Integer operator~()const;
		friend Integer operator&(const Integer&, const Integer&);
		friend Integer operator|(const Integer&, const Integer&);
		friend Integer operator^(const Integer&, const Integer&);
		void operator&=(const Integer&);
		void operator|=(const Integer&);
		void operator^=(const Integer&);

		friend Integer operator+(const Integer&, const Integer&);
		Integer operator-()const;
		friend Integer operator-(const Integer&, const Integer&);
		friend Integer operator*(Integer, Integer);
		friend Integer operator/(const Integer&, const Integer&);
		friend Integer operator%(const Integer&, const Integer&);

		void operator+=(const Integer&);
		void operator-=(const Integer&);
		void operator*=(const Integer&);
		void operator/=(const Integer&);
		void operator%=(const Integer&);

		friend bool operator==(const Integer&, const Integer&);
		friend bool operator<(const Integer&, const Integer&);
		friend bool operator>(const Integer&, const Integer&);
		friend bool operator<= (const Integer&, const Integer&);
		friend bool operator>=(const Integer&, const Integer&);

		operator bool()const;
		bool IsOdd()const;
		bool IsEven()const;

		friend Integer Pow(Integer base, Integer pow);

	private:
		mutable Vector<uint8, _size_t> mBytes;
		void Init(const Number&);
		void Resize(_size_t)const;


		friend void _Integer_div(const Integer&, const Integer&, Integer&, Integer&);
		friend void _Integer_div(const Integer&, const Integer&, Integer&);
		friend void _Integer_mod(const Integer&, const Integer&, Integer&);
	};

	template<typename p_t, typename q_t>
	class _RationalNumber {
	public:
		_RationalNumber() {
			mP = p_t(0);
			mQ = q_t(0);
		}
		_RationalNumber(int8 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(uint8 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(int16 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(uint16 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(int32 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(uint32 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(int64 x) :mP(p_t(x)), mQ(q_t(1)) {}
		_RationalNumber(uint64 x) :mP(p_t(x)), mQ(q_t(1)) {}

		_RationalNumber(float32 x) {
			Number n(x);
			n.ToFraction(mP, mQ);
			Normalize();
		}
		_RationalNumber(float64 x) {
			Number n(x);
			n.ToFraction(mP, mQ);
			Normalize();
		}
		_RationalNumber(const char* x) {
			Number n;
			n.Init(x, false);
			n.ToFraction(mP, mQ);
			Normalize();
		}
		_RationalNumber(const std::string& x) {
			Number n;
			n.Init(x, false);
			n.ToFraction(mP, mQ);
			Normalize();
		}

		_RationalNumber(const Number& n) {
			n.ToFraction(mP, mQ);
			Normalize();
		}
		_RationalNumber(const p_t& p, const q_t& q)
			:mP(p), mQ(q) {
			Normalize();
		}
		template<typename T>
		T ToString()const {
			if (IsIndeterminateForm())return "ind";
			else if (IsInteger())return toString(mP).c_str();
			else if (IsFloat())return toString(mP) + "/" + toString(mQ);

			T res;
			if (mP < p_t(0))res = "-";
			return res + "inf";
		}
		friend std::ostream& operator<<(std::ostream& os, const _RationalNumber& n) {
			return os << n.ToString<String>();
		}

		bool IsInteger()const {
			return mQ == q_t(1);
		}
		bool IsFloat()const {
			return mQ > q_t(1);
		}
		bool IsFraction()const {
			return IsFloat();
		}
		bool IsInfinity()const {
			return mP && !mQ;
		}
		bool IsIndeterminateForm()const {
			return !mP && !mQ;
		}

		_RationalNumber operator-()const {
			return _RationalNumber(-mP, mQ);
		}

		friend _RationalNumber operator+(const _RationalNumber& x, const _RationalNumber& y) {
			q_t GCD = gcd(x.mQ, y.mQ);
			if (!GCD)return _RationalNumber(x.mP + y.mP, 0);
			q_t a = x.mQ / GCD;
			return _RationalNumber(p_t(y.mQ / GCD) * x.mP + p_t(a) * y.mP,
				a * y.mQ);
		}

		friend _RationalNumber operator-(const _RationalNumber& x, const _RationalNumber& y) {
			q_t GCD = gcd(x.mQ, y.mQ);
			if (!GCD)return _RationalNumber(x.mP + y.mP, 0);
			q_t a = x.mQ / GCD;
			return _RationalNumber(p_t(y.mQ / GCD) * x.mP - p_t(a) * y.mP,
				a * y.mQ);
		}

		friend _RationalNumber operator*(const _RationalNumber& a, const _RationalNumber& b) {
			return _RationalNumber(a.mP * b.mP, a.mQ * b.mQ);
		}

		friend _RationalNumber operator/(const _RationalNumber& a, const _RationalNumber& b) {
			return _RationalNumber(a.mP * b.mQ, a.mQ * b.mP);
		}

		void operator+=(const _RationalNumber& n) {
			q_t GCD = gcd(mQ, n.mQ);
			if (GCD) {
				q_t a = mQ / GCD;
				mP = p_t(n.mQ / GCD) * mP + p_t(a) * n.mP;
				mQ = a * n.mQ;
			}
			else mP += n.mP;
			Normalize();
		}
		void operator-=(const _RationalNumber& n) {
			q_t GCD = gcd(mQ, n.mQ), a = mQ / GCD;
			if (GCD) {
				q_t a = mQ / GCD;
				mP = p_t(n.mQ / GCD) * mP - p_t(a) * n.mP;
				mQ = a * n.mQ;
			}
			else mP -= n.mP;
			Normalize();
		}
		void operator*=(const _RationalNumber& n) {
			mP *= n.mP;
			mQ *= n.mQ;
			Normalize();
		}
		void operator/=(const _RationalNumber& n) {
			mP *= n.mQ;
			mQ *= n.mP;
			Normalize();
		}

		friend LogicState operator==(const _RationalNumber& a, const _RationalNumber& b) {
			if (a.IsIndeterminateForm() || b.IsIndeterminateForm())return notsure;
			return a.mP == b.mP && a.mQ == b.mQ;
		}
		friend LogicState operator!=(const _RationalNumber& a, const _RationalNumber& b) {
			if (a.IsIndeterminateForm() || b.IsIndeterminateForm())return notsure;
			return a.mP != b.mP || a.mQ != b.mQ;
		}
		friend LogicState operator>(const _RationalNumber& a, const _RationalNumber& b) {
			if (a.IsIndeterminateForm() || b.IsIndeterminateForm())return notsure;
			q_t GCD = gcd(a.mQ, b.mQ);
			return GCD ? p_t(a.mQ / GCD) * a.mP > p_t(b.mQ / GCD) * b.mP:a.mP > b.mP;
		}
		friend LogicState operator<(const _RationalNumber& a, const _RationalNumber& b) {
			if (a.IsIndeterminateForm() || b.IsIndeterminateForm())return notsure;
			q_t GCD = gcd(a.mQ, b.mQ);
			return GCD ? p_t(a.mQ / GCD) * a.mP < p_t(b.mQ / GCD) * b.mP : a.mP < b.mP;
		}
		friend LogicState operator>=(const _RationalNumber& a, const _RationalNumber& b) {
			if (a.IsIndeterminateForm() || b.IsIndeterminateForm())return notsure;
			q_t GCD = gcd(a.mQ, b.mQ);
			return GCD ? p_t(a.mQ / GCD) * a.mP >= p_t(b.mQ / GCD) * b.mP : a.mP >= b.mP;
		}
		friend LogicState operator<=(const _RationalNumber& a, const _RationalNumber& b) {
			if (a.IsIndeterminateForm() || b.IsIndeterminateForm())return notsure;
			q_t GCD = gcd(a.mQ, b.mQ);
			return GCD ? p_t(a.mQ / GCD) * a.mP <= p_t(b.mQ / GCD) * b.mP : a.mP >= b.mP;
		}

		p_t P()const {
			return mP;
		}
		q_t Q()const {
			return mQ;
		}
		double ToDouble()const {
			return double(P()) / double(Q());
		}
	private:
		p_t mP;
		q_t mQ;

		void Normalize() {
			if (IsIndeterminateForm())return;
			q_t GCD = gcd(q_t(abs(mP)), mQ);
			mP /= p_t(GCD);
			mQ /= GCD;
		}
	};

	typedef _RationalNumber<int64, uint64> RationalNumber;
	extern RationalNumber inf,ind;
}