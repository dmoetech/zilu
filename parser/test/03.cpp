#include "iif.h"
#include <iostream>
using namespace std;
using namespace iif;

int loopFunction(int _reserved_input_[]) {
	int l = _reserved_input_[0];

	 int n; int i;
	 i = l;
	iif_assume( l>0);

	while( i < n) {
		recordi(l);
		i++;
		
	}
	recordi(l);

	iif_assert(l>=1);

	return 0;
}

int main(int argc, char** argv) {
	 iifround = atoi(argv[1]);
	 initseed = atoi(argv[2]);
	iifContext context("test/03.cnt", loopFunction, "loopFunction", "test/03.ds");
	context.addLearner("linear");
	return context.learn("test/03.var", "test/03");
}
