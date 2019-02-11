#include "util/util.h"
#include "cudd/cudd.h"

int main()
{
  DdManager *gbm = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);

  DdNode *x0 = Cudd_bddNewVar( gbm );
  DdNode *x1 = Cudd_bddNewVar( gbm );
  DdNode *x2 = Cudd_bddNewVar( gbm );

  auto b0 = Cudd_bddXor( gbm, x0, x1 );
  auto b1 = Cudd_bddXor( gbm, b0, x2 );
  Cudd_Ref( b1 );

  Cudd_PrintDebug( gbm, b1, 2, 4 );

  Cudd_Quit(gbm);
  return 0;
}
