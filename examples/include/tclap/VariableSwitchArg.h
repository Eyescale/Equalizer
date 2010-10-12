
/****************************************************************************** 
 * 
 *  file:  VariableSwitchArg.h
 * 
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
 *  Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 *  All rights reverved.
 * 
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *  
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 *  DEALINGS IN THE SOFTWARE.  
 *  
 *****************************************************************************/ 


#ifndef TCLAP_VARIABLE_SWITCH_ARG_H
#define TCLAP_VARIABLE_SWITCH_ARG_H

#include <string>
#include <vector>

#include <tclap/Arg.h>

namespace TCLAP {

/**
 * A variable switch argument. Matches any switch argument starting with the
 * flag, and any trailing values. If the switch is set on the command line, then
 * the getValue method will return the variable part of the last argument.
 */
class VariableSwitchArg : public Arg
{
	protected:

		/**
		 * The value of the switch.
		 */
        std::string _value;

	public:

        /**
		 * VariableSwitchArg constructor.
		 * \param flag - The leading characters that identify this
		 * argument on the command line.
		 * \param desc - A description of what the argument is for or
		 * does.
		 * \param def - The default value for this switch.
		 * \param v - An optional visitor.  You probably should not
		 * use this unless you have a very good reason.
		 */
		VariableSwitchArg(const std::string& flag, 
			      const std::string& desc,
                  const std::string& def = "",
				  Visitor* v = NULL);

				  
		/**
		 * VariableSwitchArg constructor.
		 * \param flag - The leading characters that identify this
		 * argument on the command line.
		 * \param desc - A description of what the argument is for or
		 * does.
		 * \param parser - A CmdLine parser object to add this Arg to
		 * \param def - The default value for this switch.
		 * \param v - An optional visitor.  You probably should not
		 * use this unless you have a very good reason.
		 */
		VariableSwitchArg(const std::string& flag, 
			      const std::string& desc,
				  CmdLineInterface& parser,
                  const std::string& def = "",
				  Visitor* v = NULL);
				  
				  
        /**
		 * Handles the processing of the argument.
		 * This re-implements the Arg version of this method to set the
		 * _value of the argument appropriately.
		 * \param i - Pointer the the current argument in the list.
		 * \param args - Mutable list of strings. Passed
		 * in from main().
		 */
		virtual bool processArg(int* i, std::vector<std::string>& args); 

		/**
		 * Returns bool, whether or not the switch has been set.
		 */
        const std::string& getValue() const;

};

//////////////////////////////////////////////////////////////////////
//BEGIN VariableSwitchArg.cpp
//////////////////////////////////////////////////////////////////////
inline VariableSwitchArg::VariableSwitchArg(const std::string& flag, 
     		   		 const std::string& desc, 
	 		         const std::string& def, 
					 Visitor* v )
: Arg("", flag, desc, false, false, v),
  _value( def )
{ }

inline VariableSwitchArg::VariableSwitchArg(const std::string& flag, 
					const std::string& desc, 
					CmdLineInterface& parser,
    		        const std::string& def, 
					Visitor* v )
: Arg("", flag, desc, false, false, v),
  _value( def )
{ 
	parser.add( this );
}

inline const std::string& VariableSwitchArg::getValue() const { return _value; }

namespace
{
inline bool matchVariableSwitch( const std::string& arg,
                                 const std::string& basename,
                                 std::string& value )
{
    const size_t basenameLength = basename.length();
    if( arg.substr( 0, basenameLength ) != basename )
    {
        return false;
    }

    value = arg.substr( basenameLength, std::string::npos );
    return true;
}
}

inline bool VariableSwitchArg::processArg(int *i, std::vector<std::string>& args)
{
	if ( _ignoreable && Arg::ignoreRest() )
		return false;

    if( !matchVariableSwitch( args[ *i ], nameStartString() + _name, _value ))
        return false;

    _alreadySet = true;
    _checkWithVisitor();
    return true;
}

//////////////////////////////////////////////////////////////////////
//End VariableSwitchArg.cpp
//////////////////////////////////////////////////////////////////////

} //namespace TCLAP

#endif
