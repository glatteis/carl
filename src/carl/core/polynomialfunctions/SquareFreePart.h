#pragma once

#include "Derivative.h"
#include "Division.h"
#include "GCD.h"
#include "to_univariate_polynomial.h"

#include "../../converter/CoCoAAdaptor.h"
#include "../logging.h"
#include "../MultivariatePolynomial.h"
#include "../UnivariatePolynomial.h"

namespace carl {

template<typename C, typename O, typename P>
MultivariatePolynomial<C,O,P> squareFreePart(const MultivariatePolynomial<C,O,P>& polynomial) {
	CARL_LOG_DEBUG("carl.core.sqfree", "SquareFreePart of " << polynomial);
	if (polynomial.isConstant() || polynomial.isLinear()) return polynomial;

	auto s = overloaded {
	#if defined USE_COCOA
		[](const MultivariatePolynomial<mpq_class,O,P>& p){ CoCoAAdaptor<MultivariatePolynomial<mpq_class,O,P>> c({p}); return c.squareFreePart(p); },
		[](const MultivariatePolynomial<mpz_class,O,P>& p){ CoCoAAdaptor<MultivariatePolynomial<mpz_class,O,P>> c({p}); return c.squareFreePart(p); }
	#else
		[](const MultivariatePolynomial<mpq_class,O,P>& p){ return p; },
		[](const MultivariatePolynomial<mpz_class,O,P>& p){ return p; }
	#endif
	#if defined USE_GINAC
		,
		[](const MultivariatePolynomial<cln::cl_RA,O,P>& p){ return p; },
		[](const MultivariatePolynomial<cln::cl_I,O,P>& p){ return p; }
	#endif
	};
	return s(polynomial);
}

template<typename Coeff, EnableIf<is_subset_of_rationals<Coeff>> = dummy>
UnivariatePolynomial<Coeff> squareFreePart(const UnivariatePolynomial<Coeff>& p) {
	CARL_LOG_DEBUG("carl.core.sqfree", "SquareFreePart of " << p);
	if (isZero(p)) return p;
	if (p.isLinearInMainVar()) return p;
	UnivariatePolynomial<Coeff> normalized = p.coprimeCoefficients().template convert<Coeff>();
	return carl::divide(normalized, carl::gcd(normalized, derivative(normalized))).quotient;
}

template<typename Coeff, DisableIf<is_subset_of_rationals<Coeff>> = dummy>
UnivariatePolynomial<Coeff> squareFreePart(const UnivariatePolynomial<Coeff>& p) {
	CARL_LOG_DEBUG("carl.core.sqfree", "SquareFreePart of " << p);
	return carl::to_univariate_polynomial(carl::squareFreePart(MultivariatePolynomial<typename carl::UnderlyingNumberType<Coeff>::type>(p)), p.mainVar());
}

}
