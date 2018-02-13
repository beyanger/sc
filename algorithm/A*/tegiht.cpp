
#include <fstream>
#include <iostream>

using namespace std;
#define NUM 9


class TEight {
public:
	TEight() {}
	TEight(char *fname);

	virtual void Search() = 0;

protected:
	int p[NUM];
	int last, spac;
	static int q[NUM], d[], total;
	void Printf();
	bool operator==(const TEight& t);
	bool Extend(int i);
};



int TEight::q[NUM];
int TEight::d[] = {1, 3, -1, -3};
int TEight::total = 0;

TEight::TEight(char *fname) {
	ifstream fin;
	fin.open(fname, ios::in);
	if (!fin.good()) {
		cout << "open file error" << endl;
		return;
	}

	int i;
	for (i = 0; i < NUM; i++)
		fin >> p[i];

	fin.close();
	last = -1;
	total = 0;
}

void TEight::Printf() {
	ofstream fout;
	fout.open("Eight_result.txt", ios::ate|ios::app);
	fout << total++ << "t";

	for (int i = 0; i < NUM; i++)
		fout << " " << p[i];
	fout.close();

}

bool TEight::operator==(const TEight& t) {
	for (int i = 0; i < NUM; i++)
		if (t.p[i] != p[i])
			return false;
	return true;
}


bool TEight::Extend(int i) {
	if ((i == 0 && spac%3 == 2) || (i == 1&& spac>5) ||
			(i == 2 && spac%3 == 0) || (i == 3 && spac < 3))
		return false;
	int temp = spac;
	spac += d[i];
	p[temp] = p[spac];
	p[spac] = 0;
	return true;
}



template<class Type> 
class TList;

template<class Type> 
class TNode {
	friend class TList<Type>;
public:
	TNode(){}
	TNode(const Type& dat);
private:
	TNode<Type> *Next;
	Type Data;
};


template<class Type>
class TList {
public:
	TList() { Last = First = 0; Length = 0;}
	int GetLen() const { return Length;}
	int Append(const Type& t) {
		Insert(t, Length);
		return 1;
	}

	int Insert(const Type& t, int k) {
		TNode<Type> *p = new TNode<Type>;
		p->Data = t;
		if (First) {
			if (k <= 0) {
				p->Next = First;
				First = p;
			} else if (k > Length-1) {
				Last->Next = p;
				Last = Last->Next;
				Last->Next = 0;
			} else {
				k--;
				TNode<Type> *q = First;
				while (k-- > 0)
					q = q->Next;
				p->Next = q->Next;
				q->Next = p;
			}
		} else {
			First = Last = p;
			First->Next = Last->Next = 0;
		}
		Length++;
		return 1;
	}

	Type GetData(int k) {
		TNode<Type> *p = First;
		while (k-- > 0) 
			p = p->Next;
		return p->Data;
	}
	void SetData(const Type& t, int k) {
		TNode<Type> *p = First;
		while (k-- > 0) 
			p = p->Next;
		p->Data = t;
	}
private:
	TNode<Type> *First, *Last;
	int Length;
};



class TBFS: public TEight {
public:
	TBFS() {}
	TBFS(char *fname) : TEight(fname) {}
	virtual void Search() {
		TBFS t = *this;
		TList<TBFS> l;
		l.Append(t);
		int head = 0, tail = 0;
		while (head <= tail) {
			for (int i = 0; i < 4; i++) {
				t = l.GetData(head);
				if (t.Extend(i) && t.Repeat(l)>tail) {
					t.last = head;
					l.Append(t);
					tail++;
				}
				if (t.Find()) {
					t.Printl(l);
					t.Printf();
				}
			}
			head++;
		}
	
	}
private:
	void Printl(TList<TBFS>& l) {
		TBFS t = *this;
		if (t.last == -1) return;
		else {
			t = l.GetData(t.last);
			t.Printl(l);
			t.Printf();
		}
	}
	int Repeat(TList<TBFS>& l) {
		int n = l.GetLen();
		int i;
		for (i = 0; i < n; i++)
			if (l.GetData(i) == *this)
				break;
		return i;
	}
	int Find() {
		for (int i = 0; i < NUM; i++)
			if (p[i] != q[i])
				return 0;
		return 1;
	}
};


int main() {

	return 0;
}


