#ifndef HOL_CPP
#define HOL_CPP


#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include<sstream>
#include <map>
#include <functional>
#include <set>
#include <utility>


using namespace std;

typedef string FunctionSymbol;
typedef string PredicateSymbol;
typedef string Variable;


class BaseTerm;
typedef shared_ptr<BaseTerm> Term;


class BaseTerm : public enable_shared_from_this<BaseTerm> {

public:
  enum Type { TT_VARIABLE, TT_FUNCTION };
  virtual Type getType() const = 0;
  virtual void printTerm(ostream & ostr) const = 0;
  virtual ~BaseTerm() {}
};

class VariableTerm : public BaseTerm {
private:
  Variable _v;
public:
  VariableTerm(const Variable & v)
    :_v(v)
  {}

  virtual Type getType() const
  {
    return TT_VARIABLE;
  }

  const Variable & getVariable() const
  {
    return _v;
  }
  virtual void printTerm(ostream & ostr) const
  {
    ostr << _v;
  }
};

class FunctionTerm : public BaseTerm {
private:
  FunctionSymbol _f;
  vector<Term> _ops;

public:
  FunctionTerm(const FunctionSymbol & f,
           const vector<Term> & ops = vector<Term> ())
    :_f(f),
     _ops(ops)
  {}

  virtual Type getType() const
  {
    return TT_FUNCTION;
  }

  const FunctionSymbol & getSymbol() const
  {
    return _f;
  }

  const vector<Term> & getOperands() const
  {
    return _ops;
  }

  virtual void printTerm(ostream & ostr) const
  {
    ostr << _f;

    for(unsigned i = 0; i < _ops.size(); i++)
      {
    if(i == 0)
      ostr << "(";
    _ops[i]->printTerm(ostr);
    if(i != _ops.size() - 1)
      ostr << ",";
    else
      ostr << ")";
      }
  }
};

class BaseFormula;
typedef shared_ptr<BaseFormula> Formula;




class BaseFormula : public enable_shared_from_this<BaseFormula> {

public:


  enum Type { T_TRUE, T_FALSE, T_ATOM, T_NOT,
          T_AND, T_OR, T_IMP, T_IFF, T_FORALL, T_EXISTS };

  virtual void printFormula(ostream & ostr) const = 0;
  virtual Type getType() const = 0;
  virtual ~BaseFormula() {}
  bool jednakost (const BaseFormula &f1, const BaseFormula &f2){
      stringstream ss;
      string f1s,f2s;
      f1.printFormula(ss);
      ss>>f1s;
      f2.printFormula(ss);
      ss>>f2s;
      return f1s.compare(f2s)==0;
  }
  
};



class AtomicFormula : public BaseFormula {
public:
};


class LogicConstant : public AtomicFormula {

public:
};


class True : public LogicConstant {

public:
  virtual void printFormula(ostream & ostr) const
  {
    ostr << "true";
  }

  virtual Type getType() const
  {
    return T_TRUE;
  }

};


class False : public LogicConstant {

public:
  virtual void printFormula(ostream & ostr) const
  {
    ostr << "false";
  }

  virtual Type getType() const
  {
    return T_FALSE;
  }
};


class Atom : public AtomicFormula {
protected:
  PredicateSymbol _p;
  vector<Term> _ops;

public:
  Atom(const PredicateSymbol & p,
       const vector<Term> & ops = vector<Term>())
    :_p(p),
     _ops(ops)
  {}

  const PredicateSymbol & getSymbol() const
  {
    return _p;
  }

  const vector<Term> & getOperands() const
  {
    return _ops;
  }

  virtual void printFormula(ostream & ostr) const
  {
    ostr << _p;
    for(unsigned i = 0; i < _ops.size(); i++)
      {
    if(i == 0)
      ostr << "(";
    _ops[i]->printTerm(ostr);
    if(i != _ops.size() - 1)
      ostr << ",";
    else
      ostr << ")";
      }
  }

  virtual Type getType() const
  {
    return T_ATOM;
  }

};

class Equality : public Atom {
public:
  Equality(const Term & lop, const Term & rop)
    :Atom("=", vector<Term> ())
  {
    _ops.push_back(lop);
    _ops.push_back(rop);
  }

  const Term & getLeftOperand() const
  {
    return _ops[0];
  }

  const Term & getRightOperand() const
  {
    return _ops[1];
  }

  virtual void printFormula(ostream & ostr) const
  {
    _ops[0]->printTerm(ostr);
    ostr << " = ";
    _ops[1]->printTerm(ostr);
  }
};

class Disequality : public Atom {
public:
  Disequality(const Term & lop, const Term & rop)
    :Atom("~=", vector<Term> ())
  {
    _ops.push_back(lop);
    _ops.push_back(rop);
  }

  const Term & getLeftOperand() const
  {
    return _ops[0];
  }

  const Term & getRightOperand() const
  {
    return _ops[1];
  }

  virtual void printFormula(ostream & ostr) const
  {

    _ops[0]->printTerm(ostr);
    ostr << " ~= ";
    _ops[1]->printTerm(ostr);
  }
};


class UnaryConjective : public BaseFormula {
protected:
   Formula _op;
public:
  UnaryConjective(const Formula & op)
    :_op(op)
  {}

  const Formula & getOperand() const
  {
    return _op;
  }
};

class Not : public UnaryConjective {
public:
  Not(const Formula & op)
    :UnaryConjective(op)
  {}

  virtual void printFormula(ostream & ostr) const
  {
    ostr << "~";
    Type op_type = _op->getType();

    if(op_type == T_AND || op_type == T_OR ||
       op_type == T_IMP || op_type == T_IFF ||
       op_type == T_FORALL || op_type == T_EXISTS)
      ostr << "(";

    _op->printFormula(ostr);

    if(op_type == T_AND || op_type == T_OR ||
       op_type == T_IMP || op_type == T_IFF ||
       op_type == T_FORALL || op_type == T_EXISTS)
      ostr << ")";
  }

  virtual Type getType() const
  {
    return T_NOT;
  }
};


class BinaryConjective : public BaseFormula {
protected:
   Formula _op1, _op2;
public:
  BinaryConjective(const Formula & op1, const Formula & op2)
    :_op1(op1),
     _op2(op2)
  {}

  const Formula & getOperand1() const
  {
    return _op1;
  }

  const Formula & getOperand2() const
  {
    return _op2;
  }
};


class And : public BinaryConjective {
public:
  And(const Formula & op1, const Formula & op2)
    :BinaryConjective(op1, op2)
  {}

  virtual void printFormula(ostream & ostr) const
  {
    Type op1_type = _op1->getType();
    Type op2_type = _op2->getType();

    if(op1_type == T_OR || op1_type == T_IMP ||
       op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << "(";

    _op1->printFormula(ostr);

    if(op1_type == T_OR || op1_type == T_IMP ||
       op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << ")";

    ostr << " & ";

    if(op2_type == T_OR || op2_type == T_IMP ||
       op2_type == T_IFF || op2_type == T_AND ||
       op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << "(";

    _op2->printFormula(ostr);

    if(op2_type == T_OR || op2_type == T_IMP ||
       op2_type == T_IFF || op2_type == T_AND ||
       op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << ")";
  }

  virtual Type getType() const
  {
    return T_AND;
  }
 };



class Or : public BinaryConjective {
public:
  Or(const Formula & op1, const Formula & op2)
    :BinaryConjective(op1, op2)
  {}

  virtual void printFormula(ostream & ostr) const
  {

    Type op1_type = _op1->getType();
    Type op2_type = _op2->getType();

    if(op1_type == T_IMP || op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << "(";

    _op1->printFormula(ostr);

    if(op1_type == T_IMP || op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << ")";

    ostr << " | ";

    if(op2_type == T_IMP ||
       op2_type == T_IFF || op2_type == T_OR ||
       op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << "(";

    _op2->printFormula(ostr);

    if(op2_type == T_IMP ||
       op2_type == T_IFF || op2_type == T_OR ||
       op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << ")";
  }

  virtual Type getType() const
  {
    return T_OR;
  }
};


class Imp : public BinaryConjective {
public:
  Imp(const Formula & op1, const Formula & op2)
    :BinaryConjective(op1, op2)
  {}

  virtual void printFormula(ostream & ostr) const
  {

    Type op1_type = _op1->getType();
    Type op2_type = _op2->getType();

    if(op1_type == T_IMP || op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << "(";

    _op1->printFormula(ostr);

    if(op1_type == T_IMP || op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << ")";

    ostr << " => ";

    if(op2_type == T_IFF ||
       op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << "(";

    _op2->printFormula(ostr);

    if(op2_type == T_IFF ||
       op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << ")";
  }

  virtual Type getType() const
  {
    return T_IMP;
  }
};



class Iff : public BinaryConjective {

public:
  Iff(const Formula & op1, const Formula & op2)
    :BinaryConjective(op1, op2)
  {}

  virtual void printFormula(ostream & ostr) const
  {

    Type op1_type = _op1->getType();
    Type op2_type = _op2->getType();

    if(op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << "(";

    _op1->printFormula(ostr);

    if(op1_type == T_IFF ||
       op1_type == T_FORALL || op1_type == T_EXISTS)
      ostr << ")";

    ostr << " <=> ";

    if(op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << "(";

    _op2->printFormula(ostr);

    if(op2_type == T_FORALL || op2_type == T_EXISTS)
      ostr << ")";

  }

  virtual Type getType() const
  {
    return T_IFF;
  }
};


class Quantifier : public BaseFormula {
protected:
  Variable _v;
  Formula  _op;
public:
  Quantifier(const Variable & v, const Formula & op)
    :_v(v),
     _op(op)
  {}

  const Variable & getVariable() const
  {
    return _v;
  }

  const Formula & getOperand() const
  {
    return _op;
  }

};

class Forall : public Quantifier {
public:
  Forall(const Variable & v, const Formula & op)
    :Quantifier(v, op)
  {}

  virtual Type getType() const
  {
    return T_FORALL;
  }
  virtual void printFormula(ostream & ostr) const
  {
    cout << "!" << _v << " . ";

    _op->printFormula(ostr);
  }
};

class Exists : public Quantifier {
public:
  Exists(const Variable & v, const Formula & op)
    :Quantifier(v, op)
  {}

  virtual Type getType() const
  {
    return T_EXISTS;
  }

  virtual void printFormula(ostream & ostr) const
  {
    cout  << "?" << _v << " . ";

    _op->printFormula(ostr);
  }
};

inline
ostream & operator << (ostream & ostr, const Term & t)
{
  t->printTerm(ostr);
  return ostr;
}


//Formula dagrep(Formula f){
//    static map<string,Formula> proba;
//    switch (f->getType()) {
//        case BaseFormula::T_OR:
//        case BaseFormula::T_AND:
//        case BaseFormula::T_IFF:
//        case BaseFormula::T_IMP:
//        //vrati dag za levi dag za desni pa proveri za celu flu
//        break;
//        case BaseFormula::T_EXISTS:
//        case BaseFormula::T_FORALL:
//        //vla vla
//            break;
        
//    }


//}


inline
ostream & operator << (ostream & ostr, const Formula & f)
{
  f->printFormula(ostr);
  return ostr;
}

extern Formula parsed_formula;

#endif // HOL_CPP
