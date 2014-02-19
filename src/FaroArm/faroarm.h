/*
 * Ubitrack - Library for Ubiquitous Tracking
 * Copyright 2006, Technische Universitaet Muenchen, and individual
 * contributors as indicated by the @authors tag. See the 
 * copyright.txt in the distribution for a full listing of individual
 * contributors.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */
 
 /**
 * @ingroup drivers
 * @file
 * Definition of the Faro Arm Driver class.
 * It integrates the convertation of the Faro Arm values to a Ubitrack conform pose information.
 *
 * @author Christian Waechter <christian.waechter@in.tum.de>
 */

#ifndef __Ubitrack_Drivers_FaroArm_FaroArm_h_included__
#define __Ubitrack_Drivers_FaroArm_FaroArm_h_included__

#include <utDataflow/Component.h>
#include <utDataflow/ComponentFactory.h>
#include <utDataflow/PushSupplier.h>
#include <utMeasurement/Measurement.h>
#include <utUtil/Exception.h>

#include <boost/thread.hpp>

#include <string>

using namespace Ubitrack;

namespace Ubitrack { namespace Driver {


class FaroArmDriver : public Dataflow::Component
	{
	public:

	/** constructor */
	FaroArmDriver( const std::string& sName, boost::shared_ptr< Graph::UTQLSubgraph > subgraph );
	
	/** destructor */
	~FaroArmDriver( ) { stop(); };
	
	virtual void start();
	virtual void stop();

protected:
	// Output ports of the component
	Dataflow::PushSupplier< Measurement::Pose > m_outPort;
	Dataflow::PushSupplier< Measurement::Button > m_outButton1;
	Dataflow::PushSupplier< Measurement::Button > m_outButton2;

	/** method for closing  connection to Faro Arm*/
	void close ( HMODULE hArmLib, int ( *pCmmSpecific )( void * ) );
	
	/** method to convert the raw data and send it to the port */
	void sendPose ( const Measurement::Timestamp, double rawFaroData[] );
	
	/** transforms the faro-euler angles to correct rotation */
	Math::Quaternion euler2Quat( double heading, double attitude, double bank ) ;

	/** thread method */
	void threadMethod( );
	
	// pointer to the thread
	boost::shared_ptr< boost::thread > m_pThread;
	
	/** thread is running?*/
	bool m_bStop;
	
		/** thread is running?*/
	unsigned int m_frequency;
	
};

} } // namespace Ubitrack::Drivers

#endif