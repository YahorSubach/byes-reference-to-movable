
#include <cassert>
#include <iostream>
#include <vector>

#include "reference_to_movable.h"

struct S : byes::RM<S>
{
	int s;
};

int main()
{
	std::vector<S> v;
	std::vector<byes::RTM<S>> r;

	for (int i = 0; i < 100; i++)
	{
		v.push_back(S());
		v.back().s = i;
		r.push_back(byes::RTM<S>(v.back()));
	}

	for (int i = 0; i < 100; i++)
	{
		assert(r[i]->s == i);
	}

}
