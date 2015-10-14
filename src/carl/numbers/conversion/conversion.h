#pragma once

namespace carl {
	template<typename From, typename To, carl::DisableIf< std::is_same<From,To> > = dummy >
	inline To convert(const From&);

	template<typename From, typename To, carl::EnableIf< std::is_same<From,To> > = dummy >
	inline To convert(const From& n) {
		return n;
	}
}

#include "generic.h"
#include "cln_gmp.h"
#include "native.h"