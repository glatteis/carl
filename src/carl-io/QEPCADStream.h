#pragma once

#include <carl/core/Monomial.h>
#include <carl/core/MultivariatePolynomial.h>
#include <carl/core/Relation.h>
#include <carl/core/Term.h>
#include <carl/core/UnivariatePolynomial.h>
#include <carl/core/Variable.h>
#include <carl/formula/Constraint.h>
#include <carl/formula/Formula.h>
#include <carl/formula/Logic.h>
#include <carl/formula/Sort.h>

#include <iostream>
#include <sstream>
#include <type_traits>

namespace carl {

class QEPCADStream {
private:
	std::stringstream mStream;
	
	void declare(Variable v) {
		*this << "(E " << v << ") ";
	}

	template<typename Pol>
	void write(const Constraint<Pol>& c) {
		*this << c.lhs() << " " << c.relation() << " 0";
	}
	
	template<typename Pol>
	void write(const Formulas<Pol>& f, const std::string& op) {
		*this << carl::stream_joined(" " + op + " ", f);
	}
	
	template<typename Pol>
	void write(const Formula<Pol>& f) {
		switch (f.getType()) {
			case FormulaType::AND:
				write(f.subformulas(), "/\\");
				break;
			case FormulaType::OR:
				write(f.subformulas(), "\\/");
				break;
			case FormulaType::IFF:
				write(f.subformulas(), "<==>");
				break;
			case FormulaType::XOR:
				assert(false);
				break;
			case FormulaType::IMPLIES:
				assert(f.subformulas().size() == 2);
				write(f.subformulas(), "==>");
				break;
			case FormulaType::ITE:
				assert(false);
			case FormulaType::NOT:
				*this << "~ " << f.subformula();
				break;
			case FormulaType::BOOL:
				*this << f.boolean();
				break;
			case FormulaType::CONSTRAINT:
				*this << f.constraint();
				break;
			case FormulaType::VARCOMPARE:
				*this << f.variableComparison();
				break;
			case FormulaType::VARASSIGN:
				*this << f.variableAssignment();
				break;
			case FormulaType::BITVECTOR:
				CARL_LOG_ERROR("carl.qepcadstream", "Bitvectors are not supported by QEPCAD.");
				break;
			case FormulaType::TRUE:
			case FormulaType::FALSE:
				*this << f.getType();
				break;
			case FormulaType::UEQ:
				CARL_LOG_ERROR("carl.qepcadstream", "Uninterpreted equalities are not supported by QEPCAD.");
				break;
			case FormulaType::EXISTS:
			case FormulaType::FORALL:
				CARL_LOG_ERROR("carl.qepcadstream", "Printing exists or forall is not implemented yet.");
				break;
		}
	}

	void write(const Monomial::Arg& m) {
		if (m) *this << *m;
		else *this << "1";
	}
	void write(const Monomial::Content::value_type& m) {
		if (m.second == 0) *this << "1";
		else if (m.second == 1) *this << m.first;
		else {
			for (std::size_t i = 0; i < m.second; ++i) *this << " " << m.first;
		}
	}
	void write(const Monomial& m) {
		if (m.exponents().empty()) *this << "1";
		else if (m.exponents().size() == 1) *this << m.exponents().front();
		else {
			*this << " " << carl::stream_joined(" ", m.exponents());
		}
	}
	
	template<typename Coeff>
	void write(const MultivariatePolynomial<Coeff>& mp) {
		if (carl::isZero(mp)) *this << "0";
		else if (mp.nrTerms() == 1) *this << mp.lterm();
		else {
			for (auto it = mp.rbegin(); it != mp.rend(); ++it) {
				if (it != mp.rbegin()) *this << " + ";
				*this << *it;
			}
		}
	}
	
	void write(Relation r) {
		switch (r) {
			case Relation::EQ:		*this << "="; break;
			case Relation::NEQ:		*this << "/="; break;
			case Relation::LESS:	*this << "<"; break;
			case Relation::LEQ:		*this << "<="; break;
			case Relation::GREATER:	*this << ">"; break;
			case Relation::GEQ:		*this << ">="; break;
		}
	}

	template<typename Coeff>
	void write(const Term<Coeff>& t) {
		if (!t.monomial()) *this << "(" << t.coeff() << ")";
		else {
			if (carl::isOne(t.coeff())) {
				*this << t.monomial();
			} else {
				*this << "(" << t.coeff() << ") " << t.monomial();
			}
		}
	}
	
	template<typename Coeff>
	void write(const UnivariatePolynomial<Coeff>& up) {
		if (up.isConstant()) *this << up.constantPart();
		else {
			for (std::size_t i = 0; i < up.coefficients().size(); ++i) {
				if (i > 0) *this << " + ";
				std::size_t exp = up.coefficients().size() - i - 1;
				const auto& coeff = up.coefficients()[exp];
				if (exp == 0) *this << " " << coeff;
				else *this << "(" << coeff << ") " << Monomial(up.mainVar(), exp);
			}
		}
	}

	void write(const Variable& v) {
		*this << v.name();
	}
	void write(const VariableType& vt) {
		switch (vt) {
			case VariableType::VT_BOOL:				*this << "Bool"; break;
			case VariableType::VT_REAL:				*this << "Real"; break;
			case VariableType::VT_INT:				*this << "Int"; break;
			case VariableType::VT_UNINTERPRETED:	*this << "?_Uninterpreted"; break;
			case VariableType::VT_BITVECTOR:		*this << "?_Bitvector"; break;
			default:								*this << "?"; break;
		}
	}
	
	template<typename T>
	void write(T&& t) {
		mStream << t;
	}
	
public:
	QEPCADStream(): mStream() {
	}

	void initialize(const carlVariables& vars) {
		for (const auto& v: vars) {
			std::visit(overloaded {
				[this](Variable v){ declare(v); },
				[this](BVVariable v){ declare(v.variable()); },
				[this](UVariable v){ declare(v.variable()); },
			}, v);
		}
	}
	
	template<typename Pol>
	void initialize(std::initializer_list<Formula<Pol>> formulas) {
		carlVariables vars;
		for (const auto& f: formulas) {
			f.gatherVariables(vars);
		}
		initialize(vars);
	}
	
	template<typename Pol>
	void assertFormula(const Formula<Pol>& formula) {
		*this << formula;
	}
	
	template<typename T>
	QEPCADStream& operator<<(T&& t) {
		write(static_cast<const std::decay_t<T>&>(t));
		return *this;
	}
	//
	QEPCADStream& operator<<(std::ostream& (*os)(std::ostream&)) {
		write(os);
		return *this;
	}
	
	auto content() const {
		return mStream.rdbuf();
	}
};

inline std::ostream& operator<<(std::ostream& os, const QEPCADStream& qs) {
	return os << qs.content();
}

}
