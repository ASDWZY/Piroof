#pragma once

#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <list>
#include <algorithm>
#include <ctime>
#include <queue>

#define KB 1024
#define MB 1024*KB

namespace Piroof {
	typedef char int8;
	typedef unsigned char uint8;
	typedef short int16;
	typedef unsigned short uint16;
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef float float32;
	typedef double float64;

	template<typename T>
	std::string toString(const T& x) {
		std::ostringstream os;
		os << x;
		return os.str();
	}

	template<typename T>
	T stringTo(const std::string& str) {
		std::stringstream ss;
		ss << str;
		T ret; ss >> ret;
		return ret;
	}
	
	inline std::string coloredString(const std::string& msg, char fg=1, char bg=0) {
		std::string ret = "\033[3"; ret.push_back(fg + '0');
		ret += ";1m"; ret += msg;
		ret += "\033["; ret.push_back(bg + '0');
		ret.push_back('m');
		return ret;
	}

	template<typename T>
	class amr_ptr {
	public:
		T* ptr = nullptr;
		amr_ptr() {}
		amr_ptr(const T& val) :ptr(new T(val)) {}
		amr_ptr(const amr_ptr<T>& p) {
			if (p.ptr != nullptr)ptr = new T(*p.ptr);
		}
		void operator=(const amr_ptr<T>& p) {
			SafeDelete(ptr);
			if (p.ptr != nullptr)ptr = new T(*p.ptr);
		}
		~amr_ptr() {
			release();
		}
		void release() {
			//cout << "amr " << ptr << endl;
			SafeDelete(ptr);
		}
		amr_ptr operator+(int x)const {
			return amr_ptr(ptr + x);
		}
		void operator+=(int x) {
			ptr += x;
		}
		T& operator*()const {
			return *ptr;
		}
		T& operator[](int x)const {
			return *(ptr + x);
		}
		T* operator->()const {
			return ptr;
		}
		operator bool()const {
			return ptr;
		}

	};

	struct MemoryPool{
		uint8 mem[8 * MB];
		//uint8* mem = 0;
		size_t head = 0;
		
		void* allocate(size_t size) {
			void* ret = mem + head;
			head += size;
			return ret;
		}
		void deallocate(void* ptr, size_t size) {
			if ((size_t)ptr - size == size_t(head + mem))head -= size;
		}
	};
	struct MemoryPool0 {
		void* allocate(size_t size) {
			return malloc(size);
		}
		void deallocate(void* ptr, size_t size) {
			free(ptr);
		}
	};

	extern MemoryPool Pir_MemoryPool;

	template<typename T>
	struct MemoryPoolForList {
		uint8 mem[8 * MB];
		size_t head = 0;
		std::queue<T*> released;
		T* allocate() {
			if (released.size()) {
				T* ret = released.front();
				released.pop_front();
				return ret;
			}
			return head;
		}
		void deallocate(T*& ptr) {
			released.push(ptr);
			ptr = 0;
		}
	};

	template<typename T, typename _size_t = uint64>
	class Vector {
	public:
		Vector() {
			mSize = 0;
			mCapacity = 0;
			mBegin = 0;
		}

		Vector(_size_t size, const T& defaultVal = T()) {
			init(size, defaultVal);
		}

		Vector(const T* begin, _size_t size) {
			init(begin, size);
		}

		Vector(const T* begin, const T* end) {
			init(begin, end);
		}

		Vector(const Vector& x) {
			init(x);
		}

		void operator=(const Vector<T, _size_t>& x) {
			init(x);
		}

		void init(_size_t size, const T& defaultVal = T()) {
			mSize = size;
			mCapacity = mSize;
			mBegin = (T*)Pir_MemoryPool.allocate(mSize * sizeof(T));
			for (_size_t i = 0; i < mSize; i++)
				mBegin[i] = defaultVal;
		}

		void init(const T* begin, _size_t size) {
			mSize = size;
			mCapacity = mSize;
			mBegin = (T*)Pir_MemoryPool.allocate(mSize * sizeof(T));
			for (_size_t i = 0; i < mSize; i++)
				mBegin[i] = begin[i];
		}

		void init(const T* begin, const T* end) {
			mSize = end - begin;
			mCapacity = mSize;
			mBegin = (T*)Pir_MemoryPool.allocate(mSize * sizeof(T));
			for (_size_t i = 0; i < mSize; i++)
				mBegin[i] = begin[i];
		}

		void init(const Vector<T, _size_t>& x) {
			if (!x.mBegin) {
				mSize = 0;
				mCapacity = 0;
				mBegin = 0;
				return;
			}
			mSize = x.mSize;
			mCapacity = mSize;
			mBegin = (T*)Pir_MemoryPool.allocate(mSize * sizeof(T));
			//mBegin = new T[mSize];
			for (_size_t i = 0; i < mSize; i++)
				mBegin[i] = x.mBegin[i];
		}

		void clear() {
			mSize = 0;
		}

		void release() {
			Pir_MemoryPool.deallocate((void*)mBegin, mCapacity * sizeof(T));
			mBegin = 0;
			mSize = mCapacity = 0;
		}

		~Vector() {
			release();
		}

		_size_t size()const {
			return mSize;
		}

		_size_t capacity()const {
			return mCapacity;;
		}

		void resize(_size_t size, const T& defaultVal = T()) {
			_size_t os = mSize;
			mSize = size;
			if (mSize > mCapacity) {
				T* nBegin = (T*)Pir_MemoryPool.allocate(mSize * sizeof(T));
				for (_size_t i = 0; i < os; i++)
					nBegin[i] = mBegin[i];
				for (_size_t i = os; i < mSize; i++)
					nBegin[i] = defaultVal;
				if (mBegin)Pir_MemoryPool.deallocate(mBegin, sizeof(T) * mCapacity);
				mBegin = nBegin;
				mCapacity = mSize;
			}
		}

		T& back() {
			return mBegin[mSize - 1];
		}

		T back()const {
			return mBegin[mSize - 1];
		}

		T& operator[](_size_t x) {
			return mBegin[x];
		}

		T operator[](_size_t x)const {
			return mBegin[x];
		}

		void push_back(const T& val) {
			resize(mSize + 1);
			back() = val;
		}

		void emplace_back(const T& val) {
			resize(mSize + 1, val);
		}


	private:
		_size_t mSize, mCapacity;
		T* mBegin;
	};

	template<typename T, typename _size_t=uint64>
	//typedef int T;
	class List {
	public:
		struct Node {
			T data;
			Node* prev=0,*nex=0;
		};
		struct iterator {
			Node* node;
			iterator() {}
			iterator(Node* x) :node(x) {}
			bool operator==(const iterator& it)const {
				return node == it.node;
			}
			bool operator!=(const iterator& it)const {
				return node!=it.node;
			}
			T& operator*()const {
				return node->data;
			}
			iterator operator++() {
				if(node)node=node->nex;
				return *this;
			}
			iterator operator--() {
				if(node)node = node->prev;
				return *this;
			}
		};
		Node* mFirst=0, *mLast=0;
		_size_t mSize=0;
		List() {

		}
		List(const List& x) {
			*this = x;
		}
		void emplace_front(const T& data) {
			Node* x = new Node({data,0,mFirst});
			if (!mLast) {
				mLast = x;
			}
			if (mFirst)mFirst->prev = x;
			mFirst = x;
			mSize++;
		}
		void emplace_back(const T& data) {
			Node* x = new Node({ data,mLast,0 });
			if (!mFirst) {
				mFirst = x;
			}
			if (mLast)mLast->nex = x;
			mLast = x;
			mSize++;
		}
		void pop_front() {
			if (mFirst) {
				mFirst = mFirst->nex;
				if (mFirst)mFirst->prev = 0;
				else mLast = 0;
			}
			else {
				//0[0];
			}
			mSize--;
		}
		void pop_back() {
			if (mLast) {
				mLast = mLast->prev;
				if (mLast)mLast->nex = 0;
				else mFirst = 0;
			}
			else {
				//0[0];
			}
			mSize--;
		}
		void clear() {
			mSize = 0;
			Node* x = mFirst;
			while (x) {
				Node* y = x->nex;
				delete x;
				x = y;
			}
			mFirst = 0;
			mLast = 0;
		}
		_size_t size()const {
			return mSize;
		}
		T& front() {
			return mFirst->data;
		}
		const T front()const {
			return mFirst->data;
		}
		T& back() {
			return mLast->data;
		}
		const T back()const {
			return mLast->data;
		}
		iterator begin()const {
			return iterator(mFirst);
		}
		iterator end()const {
			return iterator(0);
		}
		void insert(const iterator& t, const T& data) {
			Node* x = t.node;
			if (!x) {
				emplace_back(data);
				return;
			}
			Node* tmp = new Node({ data,x,x->nex });
			if (x->nex)
				x->nex->prev = tmp;
			x->nex = tmp;
			mSize++;
		}
		void insert(const iterator& t, const iterator& a, const iterator& b) {
			Node* x = t.node;
			if (!x) {
				for (iterator it = a; it != b; ++it)
					emplace_back(*it);
				return;
			}
			iterator m = t;
			for (iterator it = a; it != b; ++it) {
				insert(m, *it);
				++m;
			}
			if(x)x->nex = t.node->nex;
		}
		void operator=(const List& x) {
			clear();
			for (iterator it = x.begin(); it != x.end(); ++it)
				emplace_back(*it);
		}
		void resize(_size_t nsize, const T& defaultVal = T()) {
			while (nsize > mSize)emplace_back(defaultVal);
			while (nsize < mSize)pop_back();
		}
	};

	template<typename T,class fn, typename _size_t>
	void sortList(List<T,_size_t>& ls,fn&& cmp) {
		/*std::priority_queue<T, std::vector<T>, cmp> q;
		for (auto& i : ls)q.push(i);
		ls.clear();
		while (q.size()) {
			ls.emplace_back(q.top());
			q.pop();
		}*/
		std::vector<T> v(ls.size());
		_size_t idx = 0;
		for (auto& i : ls)v[idx++]=i;
		sort(v.begin(), v.end(), cmp);
		ls.clear();
		for (auto& i : v)ls.emplace_back(i);
	}

	template<typename _size_t=uint64>
	class _String {
	public:
		_String() {
			init("");
		}

		_String(char x) {
			l = 0;
			mSize = 2;
			mCapacity = mSize;
			mData = (char*)Pir_MemoryPool.allocate(mSize * sizeof(char));
			mData[0] = x;
			mData[mSize - 1] = 0;
		}
		_String(const char* t) {
			init(t);
		}
		_String(const std::string& t) {
			init(t);
		}
		_String(const _String& t) {
			init(t);
		}
		void operator=(const _String& t) {
			init(t);
		}
		void release() {
			Pir_MemoryPool.deallocate(mData, mCapacity * sizeof(char));
			mData = 0;
			mCapacity = mSize = 0;
		}
		~_String() {
			release();
		}

		void init(const char* t) {
			l = 0;
			mSize = strlen(t) + 1;
			mCapacity = mSize;
			mData = (char*)Pir_MemoryPool.allocate(mSize * sizeof(char));
			for (_size_t i = 0; i < mSize; i++)mData[i] = t[i];
			mData[mSize - 1] = 0;
		}
		void init(const std::string& t) {
			l = 0;
			mSize = t.size() + 1;
			mCapacity = mSize;
			mData = (char*)Pir_MemoryPool.allocate(mSize * sizeof(char));
			for (_size_t i = 0; i < mSize; i++)mData[i] = t[i];
			mData[mSize - 1] = 0;
		}
		void init(const _String& t) {
			l = 0;
			mSize = t.mSize;
			mCapacity = mSize;
			mData = (char*)Pir_MemoryPool.allocate(mSize * sizeof(char));
			for (_size_t i = 0; i < mSize; i++)mData[i] = t.mData[i];
		}

		friend _String operator+(const _String& a, const _String& b) {
			_String c; c.resize(a.size() + b.size());
			for (_size_t i = 0; i < a.size(); i++)
				c.mData[i] = a.mData[i];
			for (_size_t i = 0; i < b.mSize; i++)
				c.mData[i + a.size()] = b.mData[i];
			return c;
		}
		void operator+=(const _String& x) {
			_size_t os = mSize - 1;
			resize(size() + x.size());
			for (_size_t i = 0; i < x.size(); i++) {
				mData[i + os] = x.mData[i];
			}
		}

		void resize(_size_t size, char defaultVal=0) {
			_size_t os = mSize - 1;
			if (l)mData[mSize - 1] = l, l = 0;
			mSize = size + 1;
			if (mSize > mCapacity) {
				char* nBegin = (char*)Pir_MemoryPool.allocate(mSize * sizeof(char));
				for (_size_t i = 0; i < os; i++)
					nBegin[i] = mData[i];
				for (_size_t i = os; i < mSize - 1; i++)
					nBegin[i] = defaultVal;
				Pir_MemoryPool.deallocate(mData, sizeof(char) * mCapacity);
				mData = nBegin;
				mCapacity = mSize;
			}
			l = mData[mSize - 1];
			mData[mSize - 1] = 0;
		}
		void clear() {
			resize(0);
		}
		_size_t size()const {
			return mSize - 1;
		}
		_size_t capacity()const {
			return mCapacity - 1;
		}
		const char* c_str()const {
			return mData;
		}
		friend std::ostream& operator<<(std::ostream& os, const _String& x) {
			return os << x.c_str();
		}
		void push_back(char x) {
			resize(mSize, x);
		}

		char& operator[](_size_t  i) {
			return mData[i];
		}
		char operator[](_size_t i)const {
			return mData[i];
		}
		char& back() {
			return mData[mSize - 2];
		}
		char back()const {
			return mData[mSize - 2];
		}

		bool operator==(const _String& o)const {
			if (o.size() != size())return false;
			for (_size_t i = 0; i < size(); i++) {
				if (o[i] != (*this)[i])return false;
			}
			return true;
		}

		bool operator!=(const _String& o)const {
			return !(*this == o);
		}
		friend bool operator<(const _String& a, const _String& b) {
			_size_t s = std::min(a.size(),b.size());
			for (_size_t i = 0; i < s; i++) {
				if (a[i] < b[i])return true;
				if (a[i] > b[i])return false;
			}
			return a.size() < b.size();
		}
		friend bool operator>(const _String& a, const _String& b) {
			_size_t s = std::min(a.size(), b.size());
			for (_size_t i = 0; i < s; i++) {
				if (a[i] > b[i])return true;
				if (a[i] < b[i])return false;
			}
			return a.size() > b.size();
		}
		friend bool operator<=(const _String& a, const _String& b) {
			_size_t s = std::min(a.size(), b.size());
			for (_size_t i = 0; i < s; i++) {
				if (a[i] < b[i])return true;
				if (a[i] > b[i])return false;
			}
			return a.size() <= b.size();
		}
		friend bool operator>=(const _String& a, const _String& b) {
			_size_t s = std::min(a.size(), b.size());
			for (_size_t i = 0; i < s; i++) {
				if (a[i] > b[i])return true;
				if (a[i] < b[i])return false;
			}
			return a.size() >= b.size();
		}
		_String substr(_size_t begin, _size_t end) {
			return "";
		}
		
		operator std::string()const {
			return mData;
		}
	private:
		char* mData;
		_size_t mSize, mCapacity;
		char l;
	};
	typedef _String<> String;

	//typedef int T;
	template<typename T>
	class Dict {
	public:
		typedef std::map<std::string, size_t> map_t;
		struct Node {
			std::string str;
			T x;
		};
		struct iterator {
			Dict* obj = 0;
			size_t i;
			iterator() {}
			iterator(const Dict* _obj, size_t _i) :obj((Dict*)((void*)_obj)) {
				i = _i;
			}

			bool operator!=(const iterator& it)const {
				return i != it.i;
			}
			Node operator*()const {
				return obj->c[i];
			}
			amr_ptr<Node> operator->()const {
				return **this;
			}
			iterator operator++() {
				i++;
				return *this;
			}
			iterator operator--() {
				i--;
				return *this;
			}
		};
		std::vector<Node> c;
		map_t m;
		Dict() {}
		void init(const std::vector<Node>& data) {
			c = data;
			size_t idx = 0;
			for (const Node& i : c) {
				m[i.str] = idx++;
			}
		}
		void operator=(const Dict<T>& o) {
			c = o.c;
			m = o.m;
		}
		iterator begin()const {
			return iterator(this, 0);
		}
		iterator end()const {
			return iterator(this, size());
		}
		T& operator[](const std::string& key) {
			if (!find(key))std::cerr << "KeyError: " + key;
			return c[m[key]].x;
		}
		T operator[](const std::string& key)const {
			if (!find(key))std::cerr << "KeyError: " + key;
			return c[((Dict*)(void*)this)->m[key]].x;
		}
		T& operator[](size_t index) {
			if (index >= c.size())std::cerr << "IndexError: " + toString(index);
			return c[index].x;
		}
		T operator[](size_t index)const {
			if (index >= c.size())std::cerr << "IndexError: " + toString(index);
			return c[index].x;
		}
		void append(const T& x) {
			std::string str = toString(c.size());
			m[str] = c.size();
			//c.emplace_back({str,x});
			c.resize(c.size() + 1);
			c.back().x = x;
			c.back().str = str;
		}
		T& insert(const std::string& key, const T& val) {
			if (find(key))
				return (*this)[key] = val;
			m[key] = c.size();
			c.emplace_back(Node{key,val});
			return c.back().x;
		}
		T& _insert(const std::string& key, const T& val)const {
			return ((Dict*)(void*)this)->insert(key, val);
		}
		void erase(const std::string& key) {
			map_t::iterator it = m.find(key);
			if (it == m.end())return;
			for (size_t i = it->second + 1; i < size(); i++) {
				m[c[i].str]--;
			}
			c.erase(c.begin() + it->second);
			m.erase(key);
		}
		void erase(const iterator& it) {
			erase((*it).str);
		}
		void rename(const std::string& oldName, const std::string& newName) {
			insert(newName, (*this)[oldName]);
			erase(oldName);
		}
		size_t size()const {
			return c.size();
		}
		void clear() {
			m.clear();
			c.clear();
		}
		bool find(const std::string& key)const {
			return m.find(key) != m.end();
		}

		Dict copy()const {
			return *this;
		}
		Dict operator+(const Dict& x)const {
			Dict ret = copy();
			for (auto i : x) {
				if (find(i.str))throw "Duplicated key: " + i.str;
				ret.insert(i.str, i.x);
			}
			return ret;
		}
		void operator+=(const Dict& x) {
			for (auto i : x) {
				if (find(i.str))throw "Duplicated key: " + i.str;
				insert(i.str, i.x);
			}
		}
		bool empty()const {
			return !size();
		}
		
	};
}
