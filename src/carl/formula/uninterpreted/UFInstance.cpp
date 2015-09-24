/**
 * @file UFInstance.cpp
 * @author Florian Corzilius <corzilius@cs.rwth-aachen.de>
 * @since 2014-10-30
 * @version 2014-10-30
 */

#include <sstream>

#include "UFInstance.h"
#include "UFInstanceManager.h"

using namespace std;

namespace carl
{
    const UninterpretedFunction& UFInstance::uninterpretedFunction() const
    {
       return UFInstanceManager::getInstance().getUninterpretedFunction( *this );
    }

    const vector<UVariable>& UFInstance::args() const
    {
       return UFInstanceManager::getInstance().getArgs( *this );
    }
    
    std::string UFInstance::toString( bool _infix, bool _friendlyNames ) const
    {
        std::stringstream ss;
        UFInstanceManager::getInstance().print( ss, *this, _infix, _friendlyNames );
        return ss.str();
    }
    
    ostream& operator<<( ostream& _os, const UFInstance& _ufun )
    {
        return UFInstanceManager::getInstance().print( _os, _ufun );
    }
}