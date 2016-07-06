#pragma once

#include <tbb/parallel_for.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>

struct TBBFunctional
{
	typedef boost::function<void (unsigned int, unsigned int)> Func;
	TBBFunctional(Func& f): func(f){}

	void operator()(const tbb::blocked_range<size_t>& r) const
	{
		TBBFunctional* nonConst = const_cast<TBBFunctional*>(this);
		nonConst->func(r.begin(), r.end() - r.begin());
	}
	Func& func;
};
