
/****************************************************************************** 
 * 
 *  file:  IgnoreUnlabeledArg.h
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


#ifndef TCLAP_IGNORE_UNLABELED_ARG_H
#define TCLAP_IGNORE_UNLABELED_ARG_H

#include <string>
#include <vector>

#include <tclap/Arg.h>

namespace TCLAP {

/**
 * An argument consuming all unlabeled arguments not parsed.
 */
class IgnoreUnlabeledArg : public Arg
{
	public:

        /**
		 * IgnoreUnlabeledArg constructor.
		 * \param v - An optional visitor.  You probably should not
		 * use this unless you have a very good reason.
		 */
		IgnoreUnlabeledArg( Visitor* v = NULL );

				  
		/**
		 * IgnoreUnlabeledArg constructor.
		 * \param parser - A CmdLine parser object to add this Arg to
		 * \param v - An optional visitor.  You probably should not
		 * use this unless you have a very good reason.
		 */
		IgnoreUnlabeledArg(CmdLineInterface& parser, Visitor* v = NULL);
				  
        /**
		 * Handles the processing of the argument.
		 * This re-implements the Arg version of this method to set the
		 * _value of the argument appropriately.
		 * \param i - Pointer the the current argument in the list.
		 * \param args - Mutable list of strings. Passed
		 * in from main().
		 */
		virtual bool processArg(int* i, std::vector<std::string>& args); 

};

//////////////////////////////////////////////////////////////////////
//BEGIN IgnoreUnlabeledArg.cpp
//////////////////////////////////////////////////////////////////////
inline IgnoreUnlabeledArg::IgnoreUnlabeledArg( Visitor* v )
: Arg("", "", "Ignoring all unlabeled arguments", false, false, v)
{ }

inline IgnoreUnlabeledArg::IgnoreUnlabeledArg(CmdLineInterface& parser,
                                              Visitor* v )
: Arg("", "", "Ignoring all unlabeled arguments", false, false, v)
{ 
	parser.add( this );
}

inline bool IgnoreUnlabeledArg::processArg(int *i,
                                           std::vector<std::string>& args)
{
    if( args[ *i ].substr( 0, 1 ) == flagStartString() ||
        args[ *i ].substr( 0, 2 ) == nameStartString( ))
    {
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
//End IgnoreUnlabeledArg.cpp
//////////////////////////////////////////////////////////////////////

} //namespace TCLAP

#endif
