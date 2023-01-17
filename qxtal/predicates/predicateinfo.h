#ifndef PREDICATEINFO_H
#define PREDICATEINFO_H

#include <cstdio>

#include "macros.h"

typedef struct PredicateInfo
	{
	int num;			// Number of entries in the rest
	int *what;			// The objects of the predicate
	int *cond;			// The conditions of the predicate
	int *values;		// The test-values of the predicate
	bool enabled;		// Whether this predicate is enabled

	void dump(void)
		{
		for (int i=0; i<num; i++)
			fprintf(stderr, "%2d: %c : %d,%d,%d\n",
					i,
					(enabled == true) ? 'Y' : 'N',
					what[i],
					cond[i],
					values[i]);
		}

	void release(void)
		{
		if (num > 0)
			{
			DELETE_ARRAY(what);
			DELETE_ARRAY(cond);
			DELETE_ARRAY(values);
			num = -1;
			enabled = false;
			}
		}

	static struct PredicateInfo nilPredicate(void)
		{
		return (struct PredicateInfo) {-1,
									   nullptr,
									   nullptr,
									   nullptr,
									   false};
		}
	} PredicateInfo;

#endif // PREDICATEINFO_H
