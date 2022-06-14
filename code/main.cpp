#include "fol.hpp"
#include "dag.hpp"


extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost
   adrese parsirane formule. */
extern Formula parsed_formula;

int main()
{
    yyparse();

  if(parsed_formula.get() != 0)
    cout << parsed_formula<<endl;

  dagcvor* fla=konvertuj(parsed_formula);
  //cout<<endl<<fla->printCvor()<<endl;
  if(fla->resi(true)){
      cout<<"formula je zadovoljiva\n";
  }else{
      cout<<"formula nije zadovoljiva\n";
  }

  //cout<<parsed_formula->getType();

//  Formula f=(((BinaryConjective*)parsed_formula.get())->getOperand1());
//  Formula g=(((BinaryConjective*)parsed_formula.get())->getOperand2());

//  cout<<f<<endl;
//  cout<<g<<endl;
//  if(f.get()==g.get()){
//      cout<<"tacno";
//  }
//  cout<<endl ;
  return 0;
}
