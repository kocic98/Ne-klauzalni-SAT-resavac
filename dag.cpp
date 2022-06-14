#include "dag.hpp"

bool stampaj_debug=false;
map<string,dagcvor*> Provera;


//primeri formula za test: ((a&b)&(~a|d|c))=>((a&b)=>(~a|d));
//(a<=>~b)&(a&b); (a|b|c)&(a|d|b);


void dagcvor::vratiNaPrethodno(){
    if(stampaj_debug) {cout<<"vracam na staro\n";}
    for(auto s=zbogMene.begin();s!=zbogMene.end();s++){
        if(stampaj_debug)cout<<"\t"<<s.base()->first->printCvor()<<" "<<s.base()->first->get_tacnost()<<" "<<s.base()->second<<endl;
        s.base()->first->tacnost=s.base()->second;
        //zbogMene.erase(s);
    }
}

void dagcvor::dodajRazlog(dagcvor* g,enum sad s){
    zbogMene.push_back(pair<dagcvor*,enum sad>(g,s));
}

bool dagcvor::resi(bool ispisi){
    set<dagcvor*> sveFormule;
    Provera.clear();
    if(propagate(T,this,false)==false){
        return false;
    }else{

        vector<dagcvor*> izabrao;
        this->dodajpodformule(sveFormule);
        for(auto pf: sveFormule){
           if(stampaj_debug) cout<<pf->printCvor()<<endl;

        }
        dagcvor* zaPropagaciju=izaberi(sveFormule);
        while(zaPropagaciju!=NULL){
            if(stampaj_debug) cout<<"izabrao sam ovu podformulu\n";
            if(zaPropagaciju->propagate(T,zaPropagaciju,false)==false){
                zaPropagaciju->vratiNaPrethodno();
                if(zaPropagaciju->propagate(NT,zaPropagaciju,false)==false){
                    zaPropagaciju->vratiNaPrethodno();
                    if(izabrao.size()==0) return false;
                    zaPropagaciju=izabrao[izabrao.size()-1];
                    izabrao.pop_back();
                    zaPropagaciju->vratiNaPrethodno();
                    while(zaPropagaciju->propagate(NT,zaPropagaciju,false)==false){
                        if(izabrao.size()==0) return false;
                        zaPropagaciju=izabrao[izabrao.size()-1];
                        izabrao.pop_back();
                        zaPropagaciju->vratiNaPrethodno();
                    }
                    zaPropagaciju=izaberi(sveFormule);
                    continue;
                }
            }
            izabrao.push_back(zaPropagaciju);
            zaPropagaciju=izaberi(sveFormule);
            if(stampaj_debug){cout<<"uspesno izabrao flu\n";}
            //if(zaPropagaciju==NULL)cout<<"ovde je vraceno2\n";
        }
    }
    //this->ispisi();
    if(ispisi)
        for(auto f:sveFormule){
            if(f->getType()==slovo){
                f->ispisi();
            }
        }

    return true;
}

dagcvor* dagcvor::izaberi(set<dagcvor*> sveFormule){
    for(auto s:sveFormule){
        if(s->tacnost==Nepoznato){
            return s;
        }
    }
    return NULL;
}



void nArno::dodajpodformule(set<dagcvor*>& niz){
    for(auto pf:podformule){
        niz.insert(pf);
        pf->dodajpodformule(niz);
    }
}

void nArno::dodaj(Formula f,enum BaseFormula::Type s ){
    if(f->getType()==s){
        dodaj(((BinaryConjective*)f.get())->getOperand1(),s);
        dodaj(((BinaryConjective*)f.get())->getOperand2(),s);
    }else{
        dagcvor* sad=konvertuj(f);

        podformule.insert(sad);


    }
}

void nArno::postaviRoditelja(dagcvor* r){
    for(auto pf:podformule){
        pf->dodajRoditelja(r);
    }
}


bool konjukcija::propagate(enum sad s,dagcvor* razlog,bool naGore) {
    if(stampaj_debug)cout<<"Krecem propagate u konjukciji: "<<this->printCvor()<<s<<tacnost<<"\n";
    if(s==NT && naGore==true){
        if(tacnost==T)return false;
        if(tacnost!=NT){
            razlog->dodajRazlog(this,tacnost);
            this->razlog=razlog;
            tacnost=NT;
            for(auto r:roditelji){//posalji svima iznad da sam netacan
                if(r->propagate(NT,razlog,true)==false){
                    //vratiNaPrethodno();
                    return false;
                }
            }
            for(auto r:podformule){//posalji deci da su nebitna jer sam netacan zbog jednog
                //cout<<"saljem nebitno: "<<r->printCvor()<<endl;
                if(r->propagate(Nebitno,razlog,false)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;

    }
    else if(s==NT && naGore==false){//netacno i ides na dole
        //trebalo bi watcherse da implementiram ali za sada rucno
        if(tacnost==T) return false;
        if(tacnost!=NT){
            razlog->dodajRazlog(this,tacnost);
            this->razlog=razlog;
            tacnost=NT;
            int broj=0;//broj razlicitih od T sinova
            dagcvor* d;
            for(auto pf:podformule){
                if(pf->get_tacnost()!=T){
                    broj++;
                    d=pf;
                }
            }
            if(broj==1){
                if(d->propagate(NT,razlog,false)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
            for(auto f:roditelji){
                if(f->propagate(NT,razlog,true)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }
    else if(s==T && naGore==false){
        if(tacnost==NT) return false;
        if(tacnost!=T){
            razlog->dodajRazlog(this,tacnost);
            this->razlog=razlog;
            tacnost=T;
            for(auto pf:podformule){
                if(pf->propagate(T,razlog,false)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
            for(auto pf:roditelji){
                if(pf->propagate(T,razlog,true)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }
    else if(s==T && naGore==true){
        if(tacnost==T)if(stampaj_debug){cout<<"vracam tacno\n";return true;}
        int broj=0;//broj podformula koje su razlicite od T
        dagcvor* zaPropagaciju;
        for(auto pf:podformule){
            if(pf->get_tacnost()!=T){
                broj++;
                zaPropagaciju=pf;
            }
        }
        if(tacnost==Nepoznato||tacnost==Nebitno){
            if(broj==0){
                razlog->dodajRazlog(this,tacnost);
                tacnost=T;
                for(auto f:roditelji){
                    if(f->propagate(T,razlog,true)==false){
                        //razlog->vratiNaPrethodno();
                        return false;
                    }
                }
            }
        }
        if(tacnost==NT){
            if(broj==1){
                if(zaPropagaciju->propagate(NT,razlog,false)==false)return false;
            }
        }
    }
    else if(s==Nebitno){
                if(tacnost==Nepoznato){
                    razlog->dodajRazlog(this,tacnost);
                    this->razlog=razlog;
                    tacnost=Nebitno;
                    for(auto pf:podformule){
                        pf->propagate(Nebitno,razlog,false);
                    }
                }
            }


    if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
}

string konjukcija::printCvor() const {
    string s;
    s.append("& ");
    for(auto pf:podformule){
       s.append("("+ pf->printCvor()+") ");
    }
    return s;
}



bool disjunkcija::propagate(enum sad s,dagcvor* razlog,bool naGore) {

    if(stampaj_debug)cout<<"Krecem propagate u disjunkciji: "<<this->printCvor()<<s<<tacnost<<"\n";
    if(s==T && naGore==true){
        if(tacnost==NT)return false;
        if(tacnost!=T){
            razlog->dodajRazlog(this,tacnost);
            this->razlog=razlog;
            tacnost=T;
            for(auto r:roditelji){//posalji svima iznad da sam tacan
                if(r->propagate(T,razlog,true)==false){
                    //vratiNaPrethodno();
                    return false;
                }
            }
            for(auto r:podformule){//posalji deci da su nebitna jer sam tacan zbog jednog
                //cout<<"saljem nebitno: "<<r->printCvor()<<endl;
                if(r->propagate(Nebitno,razlog,false)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
        }
        if(tacnost==T){
            for(auto pf:podformule){
                pf->propagate(Nebitno,razlog,false);
            }
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;

    }
    else if(s==T && naGore==false){//netacno i ides na dole
        //trebalo bi watcherse da implementiram ali za sada rucno
        if(tacnost==NT) return false;
        if(tacnost!=T){
            razlog->dodajRazlog(this,tacnost);
            this->razlog=razlog;
            tacnost=T;
            int broj=0;//broj razlicitih od NT sinova
            dagcvor* d;
            for(auto pf:podformule){
                if(pf->get_tacnost()!=NT){
                    broj++;
                    d=pf;
                }
            }
            if(broj==1){
                if(d->propagate(T,razlog,false)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
            for(auto f:roditelji){

                if(f->propagate(T,razlog,true)==false){
                    //razlog->vratiNaPrethodno();
                    return false;
                }
            }
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }
    else if(s==NT && naGore==false){

        if(tacnost==T) return false;
        if(tacnost!=NT){
            razlog->dodajRazlog(this,tacnost);
            this->razlog=razlog;
            tacnost=NT;
            for(auto pf:podformule){
                if(pf->propagate(NT,razlog,false)==false){
                    if(stampaj_debug)cout<<"vracam false\n";
                    return false;
                }
            }
            for(auto pf:roditelji){
                if(pf->propagate(NT,razlog,true)==false){
                    if(stampaj_debug)cout<<"vracam false\n";
                    return false;
                }
            }
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }
    else if(s==NT && naGore==true){
        //if(stampaj_debug)cout<<"ovaj slucaj je?\n";
        if(tacnost==NT){if(stampaj_debug)cout<<"vracam tacno\n";return true;}
        int broj=0;//broj podformula koje su razlicite od NT
        dagcvor* zaPropagaciju;
        for(auto pf:podformule){
            if(pf->get_tacnost()!=NT){
                broj++;
                zaPropagaciju=pf;
            }
        }
        if(tacnost==Nepoznato||tacnost==Nebitno){
            if(broj==0){
                razlog->dodajRazlog(this,tacnost);
                tacnost=NT;
                for(auto f:roditelji){
                    if(f->propagate(NT,razlog,true)==false){
                        //razlog->vratiNaPrethodno();
                        return false;
                    }
                }
            }
            //if(stampaj_debug)cout<<"ovde sam dosao\n";
        }
        if(tacnost==T){
            if(broj==1){
                if(zaPropagaciju->propagate(T,razlog,false)==false)return false;
            }
        }
    }
    else if(s==Nebitno){
                if(tacnost==Nepoznato){
                    razlog->dodajRazlog(this,tacnost);
                    this->razlog=razlog;
                    tacnost=Nebitno;
                    for(auto pf:podformule){
                        pf->propagate(Nebitno,razlog,false);
                    }
                }
            }


    if(stampaj_debug)cout<<"vracam tacno\n";return true;
}

string disjunkcija::printCvor() const {
    string s;
    s.append("| ");
    //s."&";
    //s<<podformule.size();
    for(auto pf:podformule){
       //s<<"promenljiva: "<<i;
       s.append("("+ pf->printCvor()+") ");
    }
    return s;
}

bool promenljiva::propagate(enum sad s,dagcvor*razlog,bool naGore) {
    (void)naGore;
    if(stampaj_debug)cout<<"Krecem propagate u promenljivoj: "<<this->printCvor()<<s<<"\n";
    if(s==T && tacnost!=T){
        if(tacnost==NT)return false;
        razlog->dodajRazlog(this,tacnost);
        tacnost=T;
        for(auto f:roditelji){
            if(f->propagate(T,razlog,true)==false)return false;
        }
        if(stampaj_debug){cout<<"vracam tacno\n";}
        return true;
    }else if(s==NT && tacnost!=NT){
        if(tacnost==T)return false;
        razlog->dodajRazlog(this,tacnost);
        tacnost=NT;
        for(auto f:roditelji){
            if(f->propagate(NT,razlog,true)==false)return false;
        }
        if(stampaj_debug){cout<<"vracam tacno\n";}
        return true;
    }else if(s==Nebitno&& tacnost==Nepoznato){
        //if(tacnost==NT)return false;
        razlog->dodajRazlog(this,tacnost);
        tacnost=Nebitno;
    }
    if(stampaj_debug){cout<<"vracam tacno\n";}
    return true;

}



void promenljiva::ispisi() const {
    if(tacnost==T){
        cout<<ime<<": T\n";
    }else if(tacnost==NT){
        cout<<ime<<": NT\n";
    }
}

bool negacija::propagate(enum sad s,dagcvor*razlog,bool naGore) {
    if(stampaj_debug)cout<<"Krecem propagate u negaciji: "<<this ->printCvor()<<s<<tacnost<<"\n";
    enum sad kontra;
    if(s==NT) kontra=T;
    if(s==T) kontra=NT;
    if(naGore==false){
        if(s==Nebitno){
            razlog->dodajRazlog(this,tacnost);
            tacnost=s;
            podformula->propagate(Nebitno,razlog,false);
            if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
        }
        if(tacnost==Nebitno||tacnost==Nepoznato){
            razlog->dodajRazlog(this,tacnost);
            tacnost=s;
            if(podformula->propagate(kontra,razlog,false)==false)return false;
            for(auto r:roditelji){
                if(r->propagate(tacnost,razlog,true)==false)return false;
            }
            if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
        }else if(tacnost!=s)return false;
    }else if(naGore){
        if(stampaj_debug)cout<<"ovaj je slucaj\n";
        if(tacnost==Nebitno||tacnost==Nepoznato){
            razlog->dodajRazlog(this,tacnost);
            tacnost=kontra;
            //cout<<roditelji.size()<<endl;
            for(auto r:roditelji){

                if(stampaj_debug)cout<<"krecem: "<<r->printCvor()<<endl;

                if(r->propagate(tacnost,razlog,true)==false)return false;
                if(stampaj_debug)cout<<"zavrsavam\n";
            }
            if(stampaj_debug)cout<<"vracam tacno\n";
            return true;
        }else if(tacnost==s){if(stampaj_debug)cout<<"ovo uradeo\n";return false;}



    }
    if(stampaj_debug)cout<<"vratio Tacno\n";
    return true;

}

void negacija::postaviRoditelja(dagcvor* r){
    podformula->dodajRoditelja(r);
}

void negacija::dodaj(Formula f){
    dagcvor* sad=konvertuj(((UnaryConjective*)f.get())->getOperand());
    podformula=sad;
    //ovde konvertuj lepo, vidi sta ces sa roditeljem;
    //sad->dodajRoditelja(this);

}



void binarno::dodajpodformule(set<dagcvor*>& niz){
    niz.insert(levo);
    levo->dodajpodformule(niz);
    niz.insert(desno);
    desno->dodajpodformule(niz);
}



bool implikacija::propagate(enum sad s,dagcvor*razlog,bool naGore) {
    if(stampaj_debug)cout<<"Krecem propagate u implikaciji: "<<this->printCvor()<<s<<tacnost<<"\n";
    if(naGore==false&&s==T){
        if(tacnost==NT) {if(stampaj_debug)cout<<"implikacija odozgo losa propagacija\n";return false;}
        if(tacnost!=T){
            razlog->dodajRazlog(this,tacnost);
            tacnost=T;
            for(auto r:roditelji){
                if(r->propagate(T,razlog,true)==false)return false;
            }
        }
        if(desno->get_tacnost()==T){
            if(levo->propagate(Nebitno,razlog,false)==false)return false;
        }
        if(desno->get_tacnost()==NT){
            if(levo->propagate(NT,razlog,false)==false)return false;
        }
        if(levo->get_tacnost()==T){
            if(desno->propagate(T,razlog,false)==false)return false;
        }
        if(levo->get_tacnost()==NT){
            if(desno->propagate(Nebitno,razlog,false)==false)return false;
        }

        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }else if(naGore==false&&s==NT){
        if(tacnost==T) {if(stampaj_debug)cout<<"implikacija odozgo losa propagacija\n";return false;}
        if(tacnost!=NT){
            //cout<<"dobar slucaj, sto li ne radi\n";
            razlog->dodajRazlog(this,tacnost);
            tacnost=NT;
            for(auto r:roditelji){
                if(r->propagate(NT,razlog,true)==false)return false;
            }
            //cout<<"saljem prvom tacno drugom netacno\n";
            if(levo->propagate(T,razlog,false)==false)return false;
            if(desno->propagate(NT,razlog,false)==false)return false;
        }
        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }
    else if(naGore){
        if(tacnost==Nebitno|| tacnost==Nepoznato){
            if(levo->get_tacnost()==T && desno->get_tacnost()==NT){
                razlog->dodajRazlog(this,tacnost);
                tacnost=NT;
                for(auto r:roditelji){
                    r->propagate(NT,razlog,true);
                }
            }else if(levo->get_tacnost()==NT||desno->get_tacnost()==T){
                razlog->dodajRazlog(this,tacnost);
                tacnost=T;
                for(auto r:roditelji){
                    r->propagate(T,razlog,true);
                }
            }
            if(stampaj_debug)cout<<"vracam tacno\n";return true; ;

        }else{
            if(this->propagate(tacnost,razlog,false)==false)return false;
        }


    }
    if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
}


bool ekvivalencija::propagate(enum sad s,dagcvor*razlog,bool naGore) {
    if(stampaj_debug)cout<<"Krecem propagate u ekvivalenciji: "<<this->printCvor()<<s<<tacnost<<"\n";
    if(naGore==false&&s==T){
        if(tacnost==NT) {return false;}
        if(tacnost!=T){
            razlog->dodajRazlog(this,tacnost);
            tacnost=T;
            for(auto r:roditelji){
                if(r->propagate(T,razlog,true)==false)return false;
            }
        }
        if(desno->get_tacnost()==T){
            if(levo->propagate(T,razlog,false)==false)return false;
        }
        if(desno->get_tacnost()==NT){
            if(levo->propagate(NT,razlog,false)==false)return false;
        }
        if(levo->get_tacnost()==T){
            if(desno->propagate(T,razlog,false)==false)return false;
        }
        if(levo->get_tacnost()==NT){
            if(desno->propagate(NT,razlog,false)==false)return false;
        }

        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }else if(naGore==false&&s==NT){
        if(tacnost==T){ return false;}
        if(tacnost!=NT){
            razlog->dodajRazlog(this,tacnost);
            tacnost=NT;
            for(auto r:roditelji){
                if(r->propagate(NT,razlog,true)==false)return false;
            }
        }

        if(levo->get_tacnost()!=Nebitno && levo->get_tacnost()!=Nepoznato){
            if(levo->get_tacnost()==T){
                if(desno->propagate(NT,razlog,false)==false)return false;
            }
            else if(desno->propagate(T,razlog,false)==false)return false;
        }
        if(desno->get_tacnost()!=Nebitno && desno->get_tacnost()!=Nepoznato){
            if(desno->get_tacnost()==T){
                if(levo->propagate(NT,razlog,false)==false)return false;
            }
            else if(levo->propagate(T,razlog,false)==false)return false;
        }

        if(stampaj_debug)cout<<"vracam tacno\n";return true; ;
    }
    else if(naGore){
        if(tacnost==Nebitno|| tacnost==Nepoznato){
            if(levo->get_tacnost()==desno->get_tacnost()){
                razlog->dodajRazlog(this,tacnost);
                tacnost=T;
                for(auto r:roditelji){
                    r->propagate(T,razlog,true);
                }
            }else if((levo->get_tacnost()==NT && desno->get_tacnost()==T)||(levo->get_tacnost()==T && desno->get_tacnost()==NT)){
                razlog->dodajRazlog(this,tacnost);
                tacnost=NT;
                for(auto r:roditelji){
                    r->propagate(NT,razlog,true);
                }
            }
            if(stampaj_debug){cout<<"vracam tacno\n";}
            return true;

        }else{
            if(this->propagate(tacnost,razlog,false)==false)return false;
        }


    }
    if(stampaj_debug)cout<<"vracam tacno\n";
    return true;
}


dagcvor* konvertuj(Formula f){
    if(stampaj_debug) cout<<"pocetak konverzije"<<endl;

    switch (f->getType()) {

    case(BaseFormula::T_AND):{
        if(stampaj_debug){f->printFormula(cout);cout<<"  konjukcija\n";}
        konjukcija* sad=new konjukcija();
        sad->dodaj(f,BaseFormula::T_AND);
        dagcvor* roditelj=sad;
        auto it=Provera.find(sad->printCvor());

        if(it!=Provera.end()){
             if(stampaj_debug)    cout<<"nasao u mapi koristim to\n";
             roditelj=it->second;
        }else{
             Provera[sad->printCvor()]=sad;
        }

        sad->postaviRoditelja(roditelj);
        if(sad!=roditelj){cout<<"obrisao jednu formulu\n";delete sad;}
        return roditelj;
    }

    case(BaseFormula::T_OR):{
        if(stampaj_debug){f->printFormula(cout);cout<<"  disjunkcija\n";}
        disjunkcija* sad=new disjunkcija();
        sad->dodaj(f,BaseFormula::T_OR);
        dagcvor* roditelj=sad;

        auto it=Provera.find(sad->printCvor());
        if(it!=Provera.end()){
             if(stampaj_debug)    cout<<"nasao u mapi koristim to\n";
             roditelj=it->second;
        }else{
             Provera[sad->printCvor()]=sad;
        }

        sad->postaviRoditelja(roditelj);
        if(sad!=roditelj)delete sad;
        return roditelj;
    }


    case(BaseFormula::T_IMP):{
        if(stampaj_debug){f->printFormula(cout);cout<<"  implikacija\n";}
        dagcvor*levo=konvertuj(((BinaryConjective*)f.get())->getOperand1());
        dagcvor*desno=konvertuj(((BinaryConjective*)f.get())->getOperand2());
        implikacija* sad=new implikacija(levo,desno);
        dagcvor *roditelj=sad;

        auto it=Provera.find(sad->printCvor());
        if(it!=Provera.end()){
             if(stampaj_debug)    cout<<"nasao u mapi koristim to\n";
             roditelj=it->second;
        }else{
             Provera[sad->printCvor()]=sad;
        }

        levo->dodajRoditelja(roditelj);
        desno->dodajRoditelja(roditelj);
        if(sad!=roditelj)delete sad;
        return roditelj;
    }


    case(BaseFormula::T_IFF):{
        if(stampaj_debug){f->printFormula(cout);cout<<"  ekvivalencija\n";}
        dagcvor*levo=konvertuj(((BinaryConjective*)f.get())->getOperand1());
        dagcvor*desno=konvertuj(((BinaryConjective*)f.get())->getOperand2());
        ekvivalencija* sad=new ekvivalencija(levo,desno);
        dagcvor *roditelj=sad;
        auto it=Provera.find(sad->printCvor());
        if(it!=Provera.end()){
             if(stampaj_debug)    cout<<"nasao u mapi koristim to\n";
             roditelj=it->second;
        }else{
             Provera[sad->printCvor()]=sad;
        }
        levo->dodajRoditelja(roditelj);
        desno->dodajRoditelja(roditelj);
        if(sad!=roditelj)delete sad;
        return roditelj;
    }


    case(BaseFormula::T_NOT):{
        if(stampaj_debug){f->printFormula(cout);cout<<"  negacija\n";}
        negacija* sad=new negacija();
        sad->dodaj(f);
        dagcvor *roditelj=sad;
        auto it=Provera.find(sad->printCvor());
        if(it!=Provera.end()){
             if(stampaj_debug)    cout<<"nasao u mapi koristim to\n";
             roditelj=it->second;
        }else{
             Provera[sad->printCvor()]=sad;
        }
        //postavi roditelja na dete sata;
        sad->postaviRoditelja(roditelj);
        if(sad!=roditelj)delete sad;

        return roditelj;

    }


    case(BaseFormula::T_ATOM):{
        if(stampaj_debug)cout<<"promenljiva\n";
        stringstream s;
        f->printFormula(s);
        string sad;
        s>>sad;
        //cout<<"promenljiva: "<<sad<<endl;
        dagcvor* n=new promenljiva(sad);
        auto it=Provera.find(n->printCvor());
        if(it!=Provera.end()){
             if(stampaj_debug)    cout<<"nasao u mapi koristim to i brisem ovo\n";
             delete n;
             n=it->second;
        }else{
             Provera[n->printCvor()]=n;
        }
        return  n;
    }
    default:
        return nullptr;
    }

    return nullptr;


    //return sad;


}
