/* 
 * File:   PolynomialFactorizationPair.tpp
 * Author: Florian Corzilius
 *
 * Created on September 5, 2014, 3:57 PM
 */

#pragma once

#include "PolynomialFactorizationPair.h"

namespace carl
{   
    template <typename P>
    std::ostream& operator<<( std::ostream& _out, const Factorization<P>& _factorization )
    {
        if( _factorization.empty() )
        {
            _out << "1";
        }
        else
        {
            for( auto polyExpPair = _factorization.begin(); polyExpPair != _factorization.end(); ++polyExpPair )
            {
                if( polyExpPair != _factorization.begin() )
                    _out << ") * (";
                else
                    _out << "(";
                _out << polyExpPair->first;
                assert( polyExpPair->second != 0 );
                if( polyExpPair->second > 1 )
                {
                    _out << "^" << polyExpPair->second;
                }
            }
            _out << ")";
        }
        return _out;
    }
    
    template<typename P>
    bool factorizationsEqual( const Factorization<P>& _factorizationA, const Factorization<P>& _factorizationB )
    {
        auto iterA = _factorizationA.begin();
        auto iterB = _factorizationB.begin();
        while( iterA != _factorizationA.end() && iterB != _factorizationB.end() )
        {
            if( iterA->second != iterB->second || !(iterA->first == iterB->first) )
                break;
            ++iterA; ++iterB;
        }
        return iterA == _factorizationA.end() && iterB == _factorizationB.end();
    }
    
    template<typename P>
    PolynomialFactorizationPair<P>::PolynomialFactorizationPair( Factorization<P>&& _factorization, P* _polynomial ):
        mHash( 0 ),
        mpPolynomial( _polynomial ),
        mFactorization( std::move( _factorization ) )
    {
        rehash();
    }
    
    template<typename P>
    void PolynomialFactorizationPair<P>::rehash()
    {
        std::lock_guard<std::recursive_mutex> lock( mMutex );
        if( mpPolynomial == nullptr )
        {
            assert( getFactorization().empty() );
            mHash = 0;
            for( auto polyExpPair = getFactorization().begin(); polyExpPair != getFactorization().end(); ++polyExpPair )
            {
                mHash = (mHash << 5) | (mHash >> (sizeof(size_t)*8 - 5));
                mHash ^= std::hash<FactorizedPolynomial<P>>()( polyExpPair->first );
                mHash ^= polyExpPair->second;
            }
        }
        else
        {
            mHash = std::hash<P>()( *mpPolynomial );
        }
    }
    
    template<typename P>
    bool operator==( const PolynomialFactorizationPair<P>& _polyFactA, const PolynomialFactorizationPair<P>& _polyFactB )
    {
        if( &_polyFactA == &_polyFactB )
            return true;
        std::lock_guard<std::recursive_mutex> lockA( _polyFactA.mMutex );
        std::lock_guard<std::recursive_mutex> lockB( _polyFactB.mMutex );
        if( _polyFactA.mpPolynomial != nullptr && _polyFactB.mpPolynomial != nullptr )
        {
            return *_polyFactA.mpPolynomial == *_polyFactB.mpPolynomial;
        }
        else
        {
            return factorizationsEqual( _polyFactA.getFactorization(), _polyFactB.getFactorization() );
        }
    }
    
    template<typename P>
    bool operator<( const PolynomialFactorizationPair<P>& _polyFactA, const PolynomialFactorizationPair<P>& _polyFactB )
    {
        if( &_polyFactA == &_polyFactB )
            return false;
        std::lock_guard<std::recursive_mutex> lockA( _polyFactA.mMutex );
        std::lock_guard<std::recursive_mutex> lockB( _polyFactB.mMutex );
        if( _polyFactA.mpPolynomial != nullptr && _polyFactB.mpPolynomial != nullptr )
        {
            return *_polyFactA.mpPolynomial < *_polyFactB.mpPolynomial;
        }
        else
        {
            auto iterA = _polyFactA.getFactorization().begin();
            auto iterB = _polyFactB.getFactorization().begin();
            while( iterA != _polyFactA.getFactorization().end() && iterB != _polyFactB.getFactorization().end() )
            {
                if( iterA->first < iterB->first )
                    return true;
                else if( iterA->first == iterB->first )
                {
                    if( iterA->second < iterB->second )
                        return true;
                    else if( iterA->second > iterB->second )
                        return false;
                }
                else
                    return false;
                ++iterA; ++iterB;
            }
            return false;
        }
    }
    
    template<typename P>
    bool canBeUpdated( const PolynomialFactorizationPair<P>& _toUpdate, const PolynomialFactorizationPair<P>& _updateWith )
    {
        if( &_toUpdate == &_updateWith )
            return false;
        std::lock_guard<std::recursive_mutex> lockA( _toUpdate.mMutex );
        std::lock_guard<std::recursive_mutex> lockB( _updateWith.mMutex );
        assert( _toUpdate.getHash() == _updateWith.getHash() && _toUpdate == _updateWith );
        if( _toUpdate.mpPolynomial == nullptr && _updateWith.mpPolynomial != nullptr )
            return true;
        assert( _updateWith.mpPolynomial == nullptr || (*_toUpdate.mpPolynomial) == (*_updateWith.mpPolynomial) );
        return !factorizationsEqual( _toUpdate.getFactorization(), _updateWith.getFactorization() );
    }

    template<typename P>
    void update( PolynomialFactorizationPair<P>& _toUpdate, PolynomialFactorizationPair<P>& _updateWith )
    {
        assert( canBeUpdated( _toUpdate, _updateWith ) ); // This assertion only ensures efficient use this method.
        assert( &_toUpdate != &_updateWith );
        std::lock_guard<std::recursive_mutex> lockA( _toUpdate.mMutex );
        std::lock_guard<std::recursive_mutex> lockB( _updateWith.mMutex );
        if( _toUpdate.mpPolynomial == nullptr && _updateWith.mpPolynomial != nullptr )
            _toUpdate.mpPolynomial = _updateWith.mpPolynomial;
        if( !factorizationsEqual( _toUpdate.getFactorization(), _updateWith.getFactorization() ) )
        {
            // Calculating the gcd refines both factorizations to the same factorization
            gcd( _toUpdate, _updateWith );
        }
        _toUpdate.rehash();
    }
    
    template<typename P>
    Factorization<P> gcd( const PolynomialFactorizationPair<P>& _pfPairA, const PolynomialFactorizationPair<P>& _pfPairB, bool& _pfPairARefined, bool& _pfPairBRefined )
    {
        if( &_pfPairA == &_pfPairB )
            return _pfPairA.getFactorization();
        std::lock_guard<std::recursive_mutex> lockA( _pfPairA.mMutex );
        std::lock_guard<std::recursive_mutex> lockB( _pfPairB.mMutex );
        Factorization<P> result;
        // TODO: implementation
        Factorization<P> factorizationA = _pfPairA.getFactorization();
        Factorization<P> factorizationB = _pfPairB.getFactorization();
        auto factorA = factorizationA.begin();
        auto factorB = factorizationB.begin();

        while( factorA != factorizationA.end() && factorB != factorizationB.end() )
        {
            if( factorA->first == factorB->first )
            {
                //Common factor found
                size_t exponent = factorA->second < factorB->second ? factorA->second : factorB->second;
                result.insert( result.end(), std::pair<FactorizedPolynomial<P>, size_t>( factorA->first, exponent ) );
                factorA++;
                factorB++;
            }
            else
            {
                //TODO irreducible?
                //Compute GCD of factors
                P polA = *factorA->first.content().mpPolynomial;
                P polB = *factorB->first.content().mpPolynomial;
                P polGCD( carl::gcd( polA, polB ) );
                std::cout << "GCD of " << polA << " and " << polB << ": " << polGCD << std::endl;
                Cache<PolynomialFactorizationPair<P>>& cache = factorA->first.mrCache;
                FactorizedPolynomial<P> gcdResult( polGCD, cache );
                if ( !gcdResult.isOne() )
                {
                    //New common factor
                    //Compute remainders
                    P remainA, remainB;
                    bool correct = polA.divideBy( polGCD, remainA );
                    assert( correct );
                    correct = polB.divideBy( polGCD, remainB );
                    assert( correct );
                    std::cout << "Remainders: " << remainA << " and " << remainB << std::endl;
                    size_t exponent = factorA->second < factorB->second ? factorA->second : factorB->second;
                    result.insert( result.end(), std::pair<FactorizedPolynomial<P>, size_t>( gcdResult,  exponent ) );

                    //Set new factorizations
                    if (remainA != 1)
                    {
                        //TODO encapsulate + rehash
                        Factorization<P> factorsA = factorA->first.content().mFactorization;
                        assert( factorsA.size() == 1 );
                        _pfPairARefined = true;
                        factorsA.clear();
                        factorsA.insert ( factorsA.end(), std::pair<FactorizedPolynomial<P>, size_t>( gcdResult, 1 ) );
                        factorsA.insert ( factorsA.end(), std::pair<FactorizedPolynomial<P>, size_t>( FactorizedPolynomial<P>( remainA, cache ), 1 ) );
                        std::cout << "New factorization: " << factorA->first << std::endl;
                    }
                    if (remainB != 1)
                    {
                        Factorization<P> factorsB = factorB->first.content().mFactorization;
                        assert( factorsB.size() == 1 );
                        _pfPairBRefined = true;
                        factorsB.clear();
                        factorsB.insert ( factorsB.end(), std::pair<FactorizedPolynomial<P>, size_t>( gcdResult, 1));
                        factorsB.insert ( factorsB.end(), std::pair<FactorizedPolynomial<P>, size_t>( FactorizedPolynomial<P>( remainB, cache ), 1));
                        std::cout << "New factorization: " << factorB->first << std::endl;
                    }
                }
                if (factorA->first < factorB->first)
                    factorA++;
                else
                    factorB++;
            }
        }
        std::cout << "General GCD of " << _pfPairA << " and " << _pfPairB << ": " << result << std::endl;
        return result;
    }
    
    template <typename P>
    std::ostream& operator<<(std::ostream& _out, const PolynomialFactorizationPair<P>& _pfPair)
    {
        if( _pfPair.getFactorization().size() == 1 && _pfPair.getFactorization().begin()->second )
        {
            assert( _pfPair.getFactorization().begin()->second == 1 );
            assert( _pfPair.mpPolynomial != nullptr );
            _out << *_pfPair.mpPolynomial;
        }
        else
        {   
            _out << _pfPair.getFactorization();
        }
        return _out;
    }
    
} // namespace carl