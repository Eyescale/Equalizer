/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.h>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "server.h"

#include "display.h"
#include "resources.h"

#include "../config.h"
#include "../global.h"
#include "../loader.h"
#include "../node.h"
#include "../server.h"

<<<<<<< HEAD
#include <co/co.h>

=======
>>>>>>> 51013caee32bb2307fd2c650fd1bc2c04a130d99
#include <fstream>

namespace eq
{
namespace server
{
namespace config
{

ServerPtr Server::configure( const std::string& session )
{
   Global::instance()->setConfigFAttribute( Config::FATTR_VERSION, 1.2f );
   ServerPtr server = new server::Server;

   Config* config = new Config( server );
   config->setName( session + " autoconfig" );

   if( !Resources::discover( config, session ))
       return 0;

   if( config->getNodes().size() > 1 )
       // add server connection for cluster configs
       server->addConnectionDescription( new ConnectionDescription );

   Display::discoverLocal( config );
   const Compounds compounds = Loader::addOutputCompounds( server );
   if( compounds.empty( ))
       return 0;

   const Channels channels = Resources::configureSourceChannels( config );
   Resources::configure( compounds, channels );

   configureForBenchmark( config, session );

   std::ofstream configFile;
   const std::string filename = session + ".auto.eqc";
   configFile.open( filename.c_str( ));
   configFile << co::base::indent << Global::instance() << *server
<< co::base::exdent << std::endl;
   configFile.close();

   return server;
}

void Server::configureForBenchmark( Config* config, const std::string& session )
{
   EQINFO << "Session:" << session << std::endl;

   std::string affState;
   std::string networkType;
   std::string protocol;

   std::string sessionCopy = session.c_str(); // strtok changes the underlying memory

   char *token = strtok( (char *)sessionCopy.c_str(), "-" );
   if( !token )
       return;

   affState = token;

   if( affState != "AffEnabled" || affState != "AffDisabled" || affState != "WrongAffEnabled" )
       return;

   token = strtok (NULL, "-");
   if( !token )
          return;

   networkType = token;

   if( networkType != "TenGig" || networkType != "Infiniband" )
         return;

   token = strtok (NULL, "-");
   if( !token )
          return;

   protocol = token;

   if( protocol != "TCPIP" || protocol != "SDP" || protocol != "RDMA" )
         return;

   const Nodes& nodes = config->getNodes();

   EQINFO << "Session:" << session << std::endl;

   if( affState == "AffEnabled" ||  affState == "WrongAffEnabled" )
   {
      int32_t affEnabledCoreArr[3] = { 2, 3, 8 };
      int32_t wrongAffEnabledCoreArr[3] = { 8, 9, 2 };
      int32_t *affCoreArr = NULL;

      for(NodesCIter it = nodes.begin(); it != nodes.end(); ++it )
      {
          const Pipes& pipes = (*it)->getPipes();
          const int nbOfPipes = pipes.size();
          EQINFO << "Number of pipes:" << nbOfPipes << std::endl;

          if( nbOfPipes == 3 )
          {
              if( affState == "AffEnabled" )
                  affCoreArr = affEnabledCoreArr;
              else if( affState == "WrongAffEnabled"  )
                  affCoreArr = wrongAffEnabledCoreArr;

              pipes[0]->setIAttribute(Pipe::IATTR_HINT_AFFINITY, affCoreArr[0] );
              pipes[1]->setIAttribute(Pipe::IATTR_HINT_AFFINITY, affCoreArr[1] );
              pipes[2]->setIAttribute(Pipe::IATTR_HINT_AFFINITY, affCoreArr[2] );
          }

          EQINFO << "Number of pipes:" << nbOfPipes << std::endl;
      }
   }

   if( networkType == "TenGig" )
   {
	   for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
       {
           const co::ConnectionDescriptions& descriptions = (*i)->getConnectionDescriptions();
    	   EQINFO << "Descriptor size:" << descriptions.size() << std::endl;
           if( (*i)->isApplicationNode( ) ) // Is appnode
           {
               co::ConnectionDescriptionPtr desc = descriptions[0];
               desc->setHostname( std::string( "node01t.cluster") ); // Hardcoded first node
        	   continue;
           }

           co::ConnectionDescriptionPtr desc = new co::ConnectionDescription();

           std::string hostname = (*i)->getHost().c_str(); // strtok changes the underlying memory
           hostname = strtok( (char *)hostname.c_str(), ".");

           desc->setHostname( hostname + "t.cluster" );
           desc->type = co::CONNECTIONTYPE_TCPIP;

           (*i)->addConnectionDescription( desc );
       }
   }
   else if( networkType == "Infiniband" )
   {
		for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
		{
			const co::ConnectionDescriptions& descriptions = (*i)->getConnectionDescriptions();
			EQINFO << "Descriptor size:" << descriptions.size() << std::endl;
			if( (*i)->isApplicationNode( ) ) // Is appnode
			{
				co::ConnectionDescriptionPtr desc = descriptions[0];
				desc->setHostname( std::string( "node01i.cluster") ); // Hardcoded first node
				continue;
			}

			co::ConnectionDescriptionPtr desc( new co::ConnectionDescription() );

			std::string hostname = (*i)->getHost().c_str(); // strtok changes the underlying memory
			hostname = strtok( (char *)hostname.c_str(), ".");

			desc->setHostname( hostname + "i.cluster" );

			co::ConnectionType connType = co::CONNECTIONTYPE_TCPIP;
			if( protocol == "TCPIP" )
				connType = co::CONNECTIONTYPE_TCPIP;
			else if( protocol == "SDP" )
				connType = co::CONNECTIONTYPE_SDP;
			else if( protocol == "RDMA" )
				connType = co::CONNECTIONTYPE_RDMA;

	        desc->type = connType;

			(*i)->addConnectionDescription( desc );
      }
   }
}


}
}
}
