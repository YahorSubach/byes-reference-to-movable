
#include <cassert>
#include <iostream>

#include "live_ref.h"

struct S : byes::LiveRefTarget<S>
{
	int s;
};

int main()
{
	S s;
	s.s = 1;
	const S cs{};
	byes::LiveRef<S> ref1 = s;
	byes::LiveRef<const S> ref2 = s;
	byes::LiveRef<const S> ref3 = ref2;

	s.byes::LiveRefTarget<S>::~LiveRefTarget<S>();
	int a = 1;


	//byes::LiveRef<S> ref2 = cs;
	//byes::LiveRef<const S> ref2 = cs;

	//byes::LiveRef<S> ref1_copy = cs;

}
