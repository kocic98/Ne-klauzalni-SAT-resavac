#ifndef DAG_HPP
#define DAG_HPP
#include "fol.hpp"


class dagcvor{
protected:

    enum sad {NT,T, Nepoznato,Nebitno} tacnost;


    dagcvor* razlog;
    vector<pair<dagcvor*,enum sad>> zbogMene;
    vector<dagcvor*> roditelji;
public:
    dagcvor():tacnost(Nepoznato){}
    void vratiNaPrethodno();
    void dodajRazlog(dagcvor* g,enum sad s);
    void dodajRoditelja(dagcvor* r){roditelji.push_back(r);}
    int get_tacnost(){return tacnost;}
    enum tip{i,ili,sledi, ne, ekvivalentno,slovo};
    virtual tip getType() const = 0;
    virtual string printCvor() const = 0;
    virtual void ispisi() const = 0;
    virtual ~dagcvor() {};
    virtual bool propagate(enum sad s,dagcvor* razlog,bool naGore) = 0;
    virtual void dodajpodformule(set<dagcvor*>& niz)=0;

    bool resi(bool ispisi);

    dagcvor* izaberi(set<dagcvor*> sveFormule);
};

dagcvor* konvertuj(Formula f);

struct cmp {

    bool operator()(dagcvor* a, dagcvor* b){
        return a->printCvor().compare(b->printCvor())<0;
    }

};

class nArno:public dagcvor{
protected:
    set<dagcvor*,cmp> podformule;
public:
    void ispisi() const {};
    void dodajpodformule(set<dagcvor*>& niz);

    void dodaj(Formula f, enum BaseFormula::Type s);
    void postaviRoditelja(dagcvor* r);

};


class konjukcija:public nArno{

public:

    virtual tip getType() const{return i;    }
    virtual string printCvor() const ;
    bool propagate(enum sad s,dagcvor* razlog,bool naGore);

};

class disjunkcija:public nArno{

public:
    virtual tip getType() const{return ili;}
    bool propagate(enum sad s,dagcvor* razlog,bool naGore);
    string printCvor() const;
};

class promenljiva:public dagcvor{
private:
    string ime;
public:
    void ispisi() const ;
    void dodajpodformule(set<dagcvor*>& niz){(void)niz;return;}
    bool propagate(enum sad s,dagcvor*razlog,bool naGore);
    promenljiva(string s):ime(s){};
    tip getType() const{return slovo;}
    string printCvor() const {return ime;}

};

class negacija:public dagcvor{
private:
    dagcvor* podformula;

public:
    void ispisi() const {};
    negacija(){};
    void postaviRoditelja(dagcvor* r);
    void dodajpodformule(set<dagcvor*>& niz){
        niz.insert(podformula);
        podformula->dodajpodformule(niz);
    }
    bool propagate(enum sad s,dagcvor*razlog,bool naGore);
    virtual tip getType() const{return ne;}
    virtual string printCvor() const {
        string s;
        s.append("~ ");
        s.append("("+ podformula->printCvor()+") ");
        return s;
    }

    void dodaj(Formula f);

};

class binarno:public dagcvor{
protected:
    dagcvor* levo,*desno;
public:
    void ispisi() const {};
    void dodajpodformule(set<dagcvor*>& niz);
};

class implikacija:public binarno{
public:
     implikacija(dagcvor* l, dagcvor* d){
         levo=l;
         desno=d;
     }
     bool propagate(enum sad s,dagcvor*razlog,bool naGore);
     virtual tip getType() const{return sledi;}
     virtual string printCvor() const {return "( "+levo->printCvor()+" => "+desno->printCvor()+" )";}
};


class ekvivalencija:public binarno{
public:
     ekvivalencija(dagcvor* l, dagcvor* d){
         levo=l;
         desno=d;
     }
     bool propagate(enum sad s,dagcvor*razlog,bool naGore);

     virtual tip getType() const{
         return ekvivalentno;
     }
     virtual string printCvor() const {
         return "( "+levo->printCvor()+" <=> "+desno->printCvor()+" )";
     }
};



#endif // DAG_HPP
