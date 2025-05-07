#include "Piroof.h"
using namespace std;
using namespace Piroof;

LogicState Piroof::notsure = LogicState::Notsure(), Piroof::contra = LogicState::Contra();
RationalNumber Piroof::inf(1, 0), Piroof::ind(0, 0);



static int compareNumber(const char* a, const char* b, int64 offset, bool nega, bool negb, uint64 m, uint64 n) {
	int ga = nega ? -1 : 1;
	if (nega && !negb)return -1;
	else if (!nega && negb)return 1;
	else if (int64(m) + offset > int64(n))return ga;
	else if (int64(m) + offset < int64(n))return -ga;
	uint64 s = std::min(m, n);
	for (uint64 i = 0; i < s; i++) {
		if (a[i] > b[i])return ga;
		else if (a[i] < b[i])return -ga;
	}
	if (offset > 0)return -ga;
	else if (offset < 0)return ga;
	else return 0;
}

static void addNumber(std::string& res, const char* a, const char* b, int64 offset, uint64 m, uint64 n) {

	if (offset < 0) {
		addNumber(res, b, a, -offset, n, m);
		return;
	}
	uint64 s = std::max(m + offset, n), ib, ia;
	char sum;
	res.resize(s + 1, '0');
	for (uint64 _i = 1; _i <= s; _i++) {
		ib = n - _i, ia = m + offset - _i;
		sum = res[s - _i + 1] - '0';
		if (0 <= ia && ia < m)sum += a[ia] - '0';
		if (0 <= ib && ib < n)sum += b[ib] - '0';
		res[s - _i + 1] = sum % 10 + '0';
		res[s - _i] += sum / 10;
	}
}

static void subNumber(std::string& res, const char* a, const char* b, int64& offset, uint64 m, uint64 n, bool& neg) {
	int com = compareNumber(a, b, offset, false, false, m, n);
	neg = false;
	if (com == 0) {
		res = "0"; return;
	}
	else if (com == -1) {
		std::swap(a, b);
		offset = -offset;
		std::swap(m, n);
		neg = true;
	}
	uint64 s = std::max(m + offset, n), ib, ia;
	char x;
	res.resize(s, '0');
	for (uint64 _i = 1; _i <= s; _i++) {
		ib = n - _i, ia = m + offset - _i;
		x = res[s - _i] - '0';
		if (0 <= ia && ia < m)x += a[ia] - '0';
		if (0 <= ib && ib < n)x -= b[ib] - '0';
		res[s - _i] = (x < 0 ? x + 10 : x) + '0';
		if (_i < s)res[s - _i - 1] -= x < 0 ? 1 : 0;
	}
	if (offset < 0) {
		res += std::string(a + m + offset, a + m);
		//neg = true;
	}
}

int64 Number::eMin = -4, Number::eMax = 6;

void Number::eRange(int64 _eMin, int64 _eMax) {
	eMin = std::min(_eMin, _eMax), eMax = std::max(_eMin, _eMax);
}

Number::Number() {
	mNegative = false;
	mPos = 0;
	mBegin = 0;
	mE = 0;
}

Number::Number(const char* str) {
	Init(str);
}

Number::Number(const std::string& str) {
	Init(str);
}

Number::Number(int8 x) {
	Init(toString(x));
}

Number::Number(uint8 x) {
	Init(toString(x));
}

Number::Number(int32 x) {
	Init(toString(x));
}

Number::Number(uint32 x) {
	Init(toString(x));
}

Number::Number(int64 x) {
	Init(toString(x));
}

Number::Number(uint64 x) {
	Init(toString(x));
}

Number::Number(float32 x) {
	Init(toString(x));
}

Number::Number(float64 x) {
	Init(toString(x));
}

void Number::Init(const std::string& str, bool E) {
	std::string temp = str;
	int s = ParseNumber(temp, mE, mNegative);
	if (s == -1 || s == 2) {
		//LOGGER.Error("Invalid number: " + str);
		return;
	}

	mBegin = 0;
	for (mPos = 0; mPos < int64(temp.size()); mPos++)
		if (temp[mPos] == '.')break;
	mStr = temp.substr(0, mPos);
	mPos++;
	if (mPos < int64(temp.size()))mStr += temp.substr(mPos, temp.size() - mPos), mPos--;
	else mPos = mStr.size();
	Normalize(E);
}

void Number::Normalize(bool E) {
	if (mBegin >= mStr.size()) {
		mE = 0, mPos = mBegin = mStr.size();
		mNegative = false;
		return;
	}
	while (mBegin < mStr.size() && mStr[mBegin] == '0')mBegin++;
	while (mStr.size() && mStr.back() == '0')mStr.pop_back();

	if (E) {
		int64 E = eNotation();
		if (eMin <= E && E <= eMax)eNo();
		else eMove(E - mE);
	}
}

std::string Number::Numbers()const {
	return mStr;
}

std::string Number::ToString()const {
	if (mBegin >= mStr.size())return "0";
	std::string ret;
	if (mNegative)ret.push_back('-');
	int64 i;
	if (mPos < int64(mStr.size())) {
		i = mPos;
		if (i <= int64(mBegin)) {
			ret += "0.";
			while (i < int64(mBegin)) {
				ret.push_back('0'); i++;
			}
		}
		else {
			ret += mStr.substr(mBegin, i - mBegin) + ".";
		}
		ret += mStr.substr(i, mStr.size() - i);
	}
	else {
		ret += mStr.substr(mBegin, mStr.size() - mBegin);
		i = mStr.size();
		while (i < mPos) {
			ret.push_back('0'); i++;
		}
	}

	if (mE)ret += "e" + toString(mE);
	return ret;
}
std::ostream& Piroof::operator<<(std::ostream& os, const Number& x) {
	return os << x.ToString();
}

LogicState Number::IsInteger()const {
	return (mPos + mE) >= int64(mStr.size());
}
LogicState Number::IsFloat()const {
	return (mPos + mE) < int64(mStr.size());
}

int64 Number::e()const {
	return mE;
}

int64 Number::MinorSize()const {
	return int64(mStr.size()) - mPos - mE;
}

void Number::ScientificNotation() {
	eMove(eNotation() - mE);
}

void Number::eNo() {
	eMove(-mE);
}

void Number::eMove(int64 x) {
	mE += x;
	mPos -= x;
}

int64 Number::eNotation() {
	if (mBegin >= mStr.size()) {
		mE = 0, mPos = mBegin = mStr.size();
		mNegative = false;
		return 0;
	}
	return mE + mPos - mBegin - 1;
}

Number Number::operator>>(int64 x)const {
	Number ret = *this;
	ret >>= x;
	return ret;
}
Number Number::operator<<(int64 x)const {
	Number ret = *this;
	ret <<= x;
	return ret;
}
void Number::operator>>=(int64 x) {
	mE -= x;
	Normalize();
}
void Number::operator<<=(int64 x) {
	mE += x;
	Normalize();
}


LogicState Piroof::operator==(const Number& a, const Number& b) {
	return compareNumber(a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
		a.mNegative, b.mNegative, a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin) == 0;
}
LogicState Piroof::operator<(const Number& a, const Number& b) {
	return compareNumber(a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
		a.mNegative, b.mNegative, a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin) == -1;
}
LogicState Piroof::operator>(const Number& a, const Number& b) {
	return compareNumber(a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
		a.mNegative, b.mNegative, a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin) == 1;
}
LogicState Piroof::operator<=(const Number& a, const Number& b) {
	return compareNumber(a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
		a.mNegative, b.mNegative, a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin) != 1;
}
LogicState Piroof::operator>=(const Number& a, const Number& b) {
	return compareNumber(a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
		a.mNegative, b.mNegative, a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin) != -1;
}


Number Piroof::operator+(const Number& a, const Number& b) {
	std::string str;
	Number res;
	if (a.mNegative == b.mNegative) {
		addNumber(str, a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
			a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin);
		res = str;
		res.mNegative = a.mNegative;
		res >>= b.mStr.size() - b.mPos - b.mE;
	}
	else {
		bool neg = false;
		int64 offset = a.Offset(b);
		subNumber(str, a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, offset,
			a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin, neg);
		res = str;
		if ((int(!neg) * 2 - 1) * (int(!a.mNegative) * 2 - 1) < 0)res.mNegative = true;
		if (neg)res >>= a.mStr.size() - a.mPos - a.mE + std::max(int64(0), -offset);
		else res >>= b.mStr.size() - b.mPos - b.mE + std::max(int64(0), -offset);
	}
	res.Normalize(false);
	return res;
}
Number Number::operator-()const {
	if (mBegin >= mStr.size())return *this;
	Number res = *this;
	res.mNegative = !mNegative;
	return res;
}

Number Piroof::operator-(const Number& a, const Number& b) {
	std::string str;
	Number res;
	if (a.mNegative != b.mNegative) {
		addNumber(str, a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, a.Offset(b),
			a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin);
		res = str;
		res.mNegative = a.mNegative;
		res >>= b.mStr.size() - b.mPos - b.mE;
	}
	else {
		bool neg = false;
		int64 offset = a.Offset(b);
		subNumber(str, a.mStr.c_str() + a.mBegin, b.mStr.c_str() + b.mBegin, offset,
			a.mStr.size() - a.mBegin, b.mStr.size() - b.mBegin, neg);
		res = str;
		if ((int(!neg) * 2 - 1) * (int(!a.mNegative) * 2 - 1) < 0)res.mNegative = true;
		if (neg)res >>= a.mStr.size() - a.mPos - a.mE + std::max(int64(0), -offset);
		else res >>= b.mStr.size() - b.mPos - b.mE + std::max(int64(0), -offset);
	}
	res.Normalize(false);
	return res;
}
Number Piroof::operator*(const Number& x, const Number& y) {

	return 0;
}

void Number::operator+=(const Number& x) {
	*this = *this + x;
}
void Number::operator-=(const Number& x) {
	*this = *this - x;
}
void Number::operator*=(const Number& x) {
	*this = *this * x;
}

int64 Number::Offset(const Number& x)const {
	return x.mStr.size() - x.mPos - (mStr.size() - mPos) - (x.mE - mE);
}


Integer::Integer() {

}

Integer::Integer(int8 x) {
	mBytes.resize(1, 0);
	mBytes[0] |= x;
	Normalize();
}

Integer::Integer(uint8 x) {
	mBytes.resize(2, 0);
	mBytes[1] |= x;
	Normalize();
}

Integer::Integer(int16 x) {
	mBytes.resize(2, 0);
	for (uint8 i = 0; i < uint8(Size()); i++) {
		mBytes[i] |= x & 0xff;
		x >>= 8;
	}
	Normalize();
}


Integer::Integer(uint16 x) {
	mBytes.resize(3, 0);
	for (uint8 i = 0; i < uint8(Size() - 1); i++) {
		mBytes[i] |= x & 0xff;
		x >>= 8;
	}
	Normalize();
}

Integer::Integer(int32 x) {
	uint8 size = 4;
	mBytes.init(size, 0);

	for (uint8 i = 0; i < size; i++) {
		mBytes[i] |= x & 0xff;
		x >>= 8;
	}
	Normalize();
}

Integer::Integer(uint32 x) {
	uint8 size = 4;
	mBytes.init(size + 1, 0);
	for (uint8 i = 0; i < size; i++) {
		mBytes[i] |= x & 0xff;
		x >>= 8;
	}
	Normalize();
}

Integer::Integer(int64 x) {
	mBytes.resize(8, 0);
	for (uint8 i = 0; i < uint8(Size()); i++) {
		mBytes[i] |= x & 0xff;
		x >>= 8;
	}
	Normalize();
}

Integer::Integer(uint64 x) {
	mBytes.resize(9, 0);
	for (uint8 i = 0; i < uint8(Size() - 1); i++) {
		mBytes[i] |= x & 0xff;
		x >>= 8;
	}
	Normalize();
}

Integer::Integer(const char* str) {
	Number n; n.Init(str, false);
	Init(n);
}

Integer::Integer(const std::string& str) {
	Number n; n.Init(str, false);
	Init(n);
}

Integer::Integer(const Number& n) {
	Init(n);
}

bool Integer::IsOdd()const {
	return Size() ? bool(mBytes[0] & 1) : false;
}

bool Integer::IsEven()const {
	return Size() ? (!bool(mBytes[0] & 1)) : true;
}

static bool string_div2(const std::string& num, std::string& res) {
	if (!num.size())return 0;
	uint8 ret = int8(num.back() - '0') & 1;
	int ad = 0, x; uint64 i = 0, j = 0;
	if (num[0] <= '1') {
		ad = 10; i++;
	}
	res.resize(num.size() - i);
	for (; i < num.size(); i++, j++) {
		x = int(num[i] - '0') + ad;
		res[j] = char(x >> 1) + '0';
		ad = (x & 1) ? 10 : 0;
	}
	return int(num.back() - '0') & 1;
}

void Integer::Init(const Number& num) {
	//0b,0x
	int64 m = -std::min(num.MinorSize(), int64(0));
	std::string numbers = num.Numbers(), tmp;
	uint8 j = 1, k = 0;

	while (m--)
		numbers.push_back('0');

	mBytes.emplace_back(uint8(0));

	while (numbers.size()) {
		uint8 x = string_div2(numbers, tmp);
		numbers = tmp;
		if (x)
			mBytes.back() |= j;
		j <<= 1; k++;
		if (k >= 8) {
			mBytes.emplace_back(uint8(0));
			j = 1, k = 0;
		}
	}

	Normalize();
	if (num < 0)*this = -*this;
}

void Integer::Normalize()const {
	uint8 size = mBytes.size();
	if (!size)return;
	uint8 _i = 1, i;
	if (mBytes.back() & 128) {
		if (mBytes.back() != uint8(0xff))return;
		while (_i < size) {
			i = size - 1 - _i;
			if (mBytes[i] != uint8(0xff)) {
				if (mBytes[i] > uint8(128))_i++;
				break;
			}
			_i++;
		}
	}
	else {
		if (mBytes.back())return;
		while (_i < size) {
			i = size - 1 - _i;
			if (mBytes[i]) {
				if (!(mBytes[i] & uint8(128)))_i++;
				break;
			}
			_i++;
		}
	}

	_i--;
	if (_i)Resize(size - _i);
}

const Integer MAXE = Integer(uint64(1e19));

template<typename T>
void CopyBytes(uint64& x, T& bytes) {
	x = 0;
	for (uint8 i = 0; i < uint8(bytes.size()); i++) {
		x |= (uint64(bytes[i]) << (8 * i));
	}
}

void Piroof::_Integer_div(const Integer& a, const Integer& b, Integer& res, Integer& mod) {
	res = 0;
	if (a < b) {
		mod = a;
		return;
	}
	Integer sum;
	Integer::_size_t cnt = 0, i, k;
	uint8 j;
	bool brk = false;
	for (Integer::_size_t _i = 0; _i < a.Size(); _i++) {
		i = a.Size() - 1 - _i;
		j = 128;
		k = 8;
		while (k--) {
			if (a.mBytes[i] & j) {
				cnt = i * 8 + k;
				brk = true;
				break;
			}
			j >>= 1;
		}
		if (brk)break;
	}
	Integer bp = b << cnt, rp = Integer(int8(1)) << cnt, tmp;
	cnt++;
	while (cnt--) {
		tmp = sum + bp;
		if (tmp <= a) {
			sum = tmp;
			res |= rp;
		}
		bp >>= 1;
		rp >>= 1;
	}
	mod = a - sum;
	res.Normalize();
}

void Piroof::_Integer_div(const Integer& a, const Integer& b, Integer& res) {
	res = 0;
	if (a < b)return;
	Integer sum, tmp;
	Integer::_size_t cnt = 0, i, k;
	uint8 j;
	bool brk = false;
	for (Integer::_size_t _i = 0; _i < a.Size(); _i++) {
		i = a.Size() - 1 - _i;
		j = 128;
		k = 8;
		while (k--) {
			if (a.mBytes[i] & j) {
				cnt = i * 8 + k;
				brk = true;
				break;
			}
			j >>= 1;
		}
		if (brk)break;
	}
	Integer bp = b << cnt, rp = Integer(int8(1)) << cnt;
	while (cnt--) {
		tmp = sum + bp;
		if (tmp <= a) {
			sum = tmp;
			res |= rp;
		}
		bp >>= 1;
		rp >>= 1;
	}
	res.Normalize();
}

void Piroof::_Integer_mod(const Integer& a, const Integer& b, Integer& mod) {
	if (a < b) {
		mod = a;
		return;
	}
	Integer sum, tmp;
	Integer::_size_t cnt = 0, i, k;
	uint8 j;
	bool brk = false;
	for (Integer::_size_t _i = 0; _i < a.Size(); _i++) {
		i = a.Size() - 1 - _i;
		j = 128;
		k = 8;
		while (k--) {
			if (a.mBytes[i] & j) {
				cnt = i * 8 + k;
				brk = true;
				break;
			}
			j >>= 1;
		}
		if (brk)break;
	}
	Integer bp = b << cnt, rp = Integer(int8(1)) << cnt;
	while (cnt--) {
		tmp = sum + bp;
		if (tmp <= a)
			sum = tmp;
		bp >>= 1;
		rp >>= 1;
	}
	mod = a - sum;
}

static int CompareInteger(const Integer& a, const Integer& b) {
	a.Normalize(); b.Normalize();
	uint8 signa = a.Sign(), signb = b.Sign();
	if (signa && !signb)return -1;
	else if (signb && !signa)return 1;
	else if (a.Size() > b.Size())return signa ? -1 : 1;
	else if (a.Size() < b.Size())return signa ? 1 : -1;
	Integer::_size_t i;
	for (Integer::_size_t _i = 0; _i < a.Size(); _i++) {
		i = a.Size() - 1 - _i;
		if (a[i] > b[i])return signa ? -1 : 1;
		else if (a[i] < b[i])return signa ? 1 : -1;
	}
	return 0;
}

std::string Integer::ToString()const {
	std::string res;
	Integer x = Sign() ? -*this : *this, tmp, val;
	uint64 num;
	while (x) {
		_Integer_div(x, MAXE, tmp, val);
		CopyBytes(num, val.mBytes);
		res = toString(num) + res;
		x = tmp;
	}
	return Sign() ? ("-" + res) : res;
}

std::string Integer::Bytes()const {
	std::string res; res.resize(Size() * 8, '0');
	Integer::_size_t i; uint8 n; int8 j;
	for (Integer::_size_t _i = 0; _i < Size(); _i++) {
		i = Size() - 1 - _i;
		n = mBytes[i];
		j = 8;
		while (j--) {
			res[_i * 8 + j] = '0' + int8(n & 1);
			n >>= 1;
		}
	}
	return res;
}

Integer::_size_t Integer::Size()const {
	return mBytes.size();
}

uint8 Integer::Sign()const {
	return mBytes.size() ? (mBytes.back() & uint8(128)) : uint8(0);
}

uint8 Integer::operator[](Integer::_size_t i)const {
	return mBytes[i];
}

std::ostream& Piroof::operator<<(std::ostream& os, const Integer& x) {
	return os << x.ToString();
}

Integer Piroof::operator<<(const Integer& x, Integer::_size_t p) {
	return 0;
	Integer res;
	if (!x)return res;
	Integer::_size_t advance = p / 8;
	p %= 8;
	if (p)res.mBytes.resize(x.Size() + advance + 1, 0);
	else res.mBytes.resize(x.Size() + advance, 0);
	uint16 ad = 0;
	for (Integer::_size_t i = 0; i < x.Size(); i++) {
		ad = (x.mBytes[i] << p) | ad >> 8;
		res.mBytes[i + advance] = uint8(ad & 0xff);
	}
	if (p) {
		res.mBytes.back() |= ad >> 8;
		if (x.Sign())
			res.mBytes.back() |= uint8(0xff);
	}
	res.Normalize();
	return res;
}

Integer Piroof::operator>>(const Integer& x, Integer::_size_t p) {
	Integer res;
	if (!x)return res;
	Integer::_size_t advance = p / 8;
	res.mBytes.resize(x.Size() - advance, 0);
	p %= 8;
	uint16 ad = 0;
	Integer::_size_t i;
	for (Integer::_size_t _i = 0; _i < x.Size() - advance; _i++) {
		i = x.Size() - 1 - _i;
		res.mBytes[i - advance] = uint8(ad & 0xff) | (x.mBytes[i] >> p);
		ad = x.mBytes[i] << 8 >> p;
	}
	res.Normalize();
	return res;
}

void Integer::operator<<=(Integer::_size_t p) {
	*this = *this << p;
}

void Integer::operator>>=(Integer::_size_t p) {
	*this = *this >> p;
}

void Integer::Resize(Integer::_size_t p)const {
	uint8 sign = Sign();
	mBytes.resize(p, sign ? uint8(0xff) : uint8(0));
	if (p)mBytes.back() |= sign;
}

Integer Integer::operator~()const {
	Integer res; res.mBytes.resize(Size());
	for (Integer::_size_t i = 0; i < Size(); i++)
		res.mBytes[i] = ~mBytes[i];
	res.Normalize();
	return res;
}

Integer Piroof::operator&(const Integer& a, const Integer& b) {
	Integer::_size_t size = a.Size();
	if (a.Size() > b.Size())b.Resize(a.Size());
	else if (a.Size() < b.Size())a.Resize(b.Size()), size = b.Size();
	Integer res; res.mBytes.resize(size);
	for (Integer::_size_t i = 0; i < size; i++)
		res.mBytes[i] = a.mBytes[i] & b.mBytes[i];
	a.Normalize();
	b.Normalize();
	res.Normalize();
	return res;
}

Integer Piroof::operator|(const Integer& a, const Integer& b) {
	Integer::_size_t size = a.Size();
	if (a.Size() > b.Size())b.Resize(a.Size());
	else if (a.Size() < b.Size())a.Resize(b.Size()), size = b.Size();
	Integer res; res.mBytes.resize(size);
	for (Integer::_size_t i = 0; i < size; i++)
		res.mBytes[i] = a.mBytes[i] | b.mBytes[i];
	a.Normalize();
	b.Normalize();
	res.Normalize();
	return res;
}

Integer Piroof::operator^(const Integer& a, const Integer& b) {
	Integer::_size_t size = a.Size();
	if (a.Size() > b.Size())b.Resize(a.Size());
	else if (a.Size() < b.Size())a.Resize(b.Size()), size = b.Size();
	Integer res; res.mBytes.resize(size);
	for (Integer::_size_t i = 0; i < size; i++)
		res.mBytes[i] = a.mBytes[i] ^ b.mBytes[i];
	a.Normalize();
	b.Normalize();
	res.Normalize();
	return res;
}

void Integer::operator&=(const Integer& x) {
	*this = *this & x;
}

void Integer::operator|=(const Integer& x) {
	*this = *this | x;
}

void Integer::operator^=(const Integer& x) {
	*this = *this ^ x;
}

Integer Piroof::operator+(const Integer& a, const Integer& b) {
	Integer::_size_t size = std::max(a.Size(), b.Size()) + 1;
	a.Resize(size); b.Resize(size);
	Integer res; res.mBytes.resize(size);

	uint16 ad = 0;
	for (Integer::_size_t i = 0; i < size; i++) {
		ad = (uint16(a.mBytes[i]) + uint16(b.mBytes[i]) + (ad >> 8));
		res.mBytes[i] = uint8(ad & 0xff);
	}
	a.Normalize();
	b.Normalize();
	res.Normalize();
	return res;
}

Integer Integer::operator-()const {
	Integer res; res.mBytes.resize(Size());
	uint16 ad = (1 << 8);
	for (Integer::_size_t i = 0; i < Size(); i++) {
		ad >>= 8;
		ad = uint8(~mBytes[i]) + ad;
		res.mBytes[i] = uint8(ad & 0xff);
	}
	return res;
}

Integer Piroof::operator-(const Integer& a, const Integer& b) {
	return a + (-b);
}

Integer Piroof::operator*(Integer a, Integer b) {
	uint8 sign = a.Sign();
	if (sign)a = -a;
	Integer res;
	while (a) {
		if (a.IsOdd()) {
			res += b;
		}
		a >>= 1;
		b <<= 1;
	}
	return sign ? -res : res;
}

Integer Piroof::operator/(const Integer& a, const Integer& b) {
	Integer res;
	uint8 signa = a.Sign(), signb = b.Sign();
	if (signa && signb)
		_Integer_div(-a, -b, res);
	else if (signa && !signb) {
		_Integer_div(-a, b, res);
		res = -res;
	}
	else if (!signa && signb) {
		_Integer_div(a, -b, res);
		res = -res;
	}
	else
		_Integer_div(a, b, res);
	return res;
}

Integer Piroof::operator%(const Integer& a, const Integer& b) {
	Integer res;
	uint8 signa = a.Sign(), signb = b.Sign();
	if (signa && signb)
		_Integer_mod(-a, -b, res);
	else if (signa && !signb) {
		_Integer_mod(-a, b, res);
		res = -res;
	}
	else if (!signa && signb) {
		_Integer_mod(a, -b, res);
		res = -res;
	}
	else
		_Integer_mod(a, b, res);
	return res;
}

void Integer::operator+=(const Integer& x) {
	*this = *this + x;
}

void Integer::operator-=(const Integer& x) {
	*this = *this - x;
}

void Integer::operator*=(const Integer& x) {
	*this = *this * x;
}

void Integer::operator/=(const Integer& x) {
	*this = *this / x;
}

void Integer::operator%=(const Integer& x) {
	*this = *this % x;
}


bool Piroof::operator==(const Integer& a, const Integer& b) {
	return CompareInteger(a, b) == 0;
}

bool Piroof::operator>(const Integer& a, const Integer& b) {
	return CompareInteger(a, b) == 1;
}

bool Piroof::operator<(const Integer& a, const Integer& b) {
	return CompareInteger(a, b) == -1;
}

bool Piroof::operator>=(const Integer& a, const Integer& b) {
	return CompareInteger(a, b) != -1;
}

bool Piroof::operator<=(const Integer& a, const Integer& b) {
	return CompareInteger(a, b) != 1;
}

Integer::operator bool()const {
	Normalize();
	return Size() && !(Size() == 1 && !mBytes[0]);
}

Integer Piroof::Pow(Integer base, Integer pow) {
	//assert(!pow.Sign());
	Integer res(int8(1));
	while (pow) {
		if (pow.IsOdd())res *= base;
		pow >>= 1;
		base *= base;
	}
	return res;
}
