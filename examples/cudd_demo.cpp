#include "util/util.h"
#include "cudd/cudd.h"
#include "cplusplus/cuddObj.hh"
#undef fail
#include <iostream>

void example1()
{
  std::cout << "=========================[    Example #1    ]=========================" << std::endl;
  DdManager *gbm = Cudd_Init( 0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );

  DdNode *x0 = Cudd_bddNewVar( gbm );
  DdNode *x1 = Cudd_bddNewVar( gbm );
  DdNode *x2 = Cudd_bddNewVar( gbm );

  auto b0 = Cudd_bddXor( gbm, x0, x1 );
  auto b1 = Cudd_bddXor( gbm, b0, x2 );
  Cudd_Ref( b1 );

  Cudd_PrintDebug( gbm, b1, 2, 4 );

  Cudd_Quit(gbm);
}

void example2()
{
  std::cout << "=========================[    Example #2    ]=========================" << std::endl;
  Cudd mgr( 0, 2 );
  mgr.makeVerbose();

  BDD x0 = mgr.bddVar();
  BDD x1 = mgr.bddVar();
  BDD x2 = mgr.bddVar();

  auto b0 = x0 ^ x1;
  auto b1 = b0 ^ x2;

  b1.print( 2, 4 );
}

int main()
{
  example1();
  example2();
  return 0;
}
