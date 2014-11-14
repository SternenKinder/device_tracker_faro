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
 * Implementation of the Faro Arm Driver to convert the values of the faro arm to ubitrack.
 *
 * @author Christian Waechter <christian.waechter@in.tum.de>
 */
 
#include "faroarm.h"

#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include "CmmSpecific.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <boost/math/quaternion.hpp>

#include <utUtil/OS.h>

#include <log4cpp/Category.hh>
static log4cpp::Category& logger( log4cpp::Category::getInstance( "Drivers.FaroArm" ) );

//Function necessary for using FARO ARM SDK
BOOL FalseSpecific ()
{
	return 0;
}

namespace Ubitrack { namespace Driver {

	FaroArmDriver::FaroArmDriver( const std::string& sName, boost::shared_ptr< Graph::UTQLSubgraph > subgraph )
		: Dataflow::Component( sName )
		, m_outPort( "Output", *this )
		, m_outButton1( "Button1", *this )
		, m_outButton2( "Button2", *this )
		, m_latencyPort("Latency", *this, boost::bind( &FaroArmDriver::receiveLatency, this, _1 ) )
		, m_bStop( true )
		, m_frequency( 50 )
		, m_synchronizer( 50 ) // assume 50 Hz for timestamp synchronization
		, m_latency( 0 )
		, m_last_pose()
	{
		stop( );
		subgraph->m_DataflowAttributes.getAttributeData ( "frequency", m_frequency );
		LOG4CPP_DEBUG( logger, "Frequency of event pushing set to " << m_frequency );
	}
	
	void FaroArmDriver::threadMethod()
	{
		LOG4CPP_DEBUG( logger, "Faro Arm thread started" );
		//Initalize the Faro Arm
		
		unsigned int counter = 0;
		
		LOG4CPP_DEBUG( logger, "Loading FaroArmUsbWrapper.dll");
		HMODULE hArmLib = LoadLibrary( "FaroArmUsbWrapper.dll" );
		if( NULL == hArmLib )
			UBITRACK_THROW( "FaroArm: Could not load FaroArmUsbWrapper.dll!" );

		int (*pCmmSpecific)(void *);
		pCmmSpecific = (BOOL (*)(VOID *))GetProcAddress( hArmLib, "CmmSpecific" );
		
		if( NULL == pCmmSpecific )
			pCmmSpecific = (BOOL (*)(VOID *))FalseSpecific;

		// Open device
		struct vOpenInfo theOpenStruct = { DLL_OPEN_CMM, NULL, L"\0", NULL };
		if( ! ( TRUE == pCmmSpecific( (void *)&theOpenStruct) ) )
		{
			UBITRACK_THROW( "FaroArm: Could not find arm!\n" );
			close( hArmLib, pCmmSpecific );
		}
		
		struct PositionUpdate theUpdateStruct;
		memset( &theUpdateStruct, 0x0, sizeof(struct PositionUpdate) );
		theUpdateStruct.nFuncType = DLL_POS_UPDATE;
		theUpdateStruct.unKey = GOOD_KEY;

		if( !( TRUE == pCmmSpecific( (void *)&theUpdateStruct) ) )
		{
			UBITRACK_THROW( "FaroArm: Could not register update struct!\n" );
			close( hArmLib, pCmmSpecific );			
		}
		
		LOG4CPP_INFO( logger, "FaroArm connected" );
		bool isButton1EventSend = false;
		while ( !m_bStop ) //Starting main loop
		{
			Measurement::Timestamp timestamp = Measurement::now();
			// subtract the approximate processing time of the cameras and DTrack (19ms)
			// (daniel) better synchronize the DTrack controller to a common NTP server and use "ts" fields directly.
			//          This should work well at least with ARTtrack2/3 cameras (not necessarily ARTtrack/TP)
			timestamp -= m_latency;

			switch( theUpdateStruct.cUpdateFlag )
			{
			case CMM_UPDATE_DETECT: // new measurement
			  timestamp = m_synchronizer.convertNativeToLocal( theUpdateStruct.lTimeStamp, timestamp );
			  sendPose( timestamp, theUpdateStruct.dPosition );
			  if ( m_outButton1.isConnected() ) {
				  if(theUpdateStruct.cFrontButton == 1) {
						if(!isButton1EventSend) {
							m_outButton1.send( Measurement::Button( timestamp, Math::Scalar < int > ( ' ' ) ) );
							isButton1EventSend = true;
						}
							
				  } else {
					  isButton1EventSend = false;
				  }
			  }
			    
			  
			  if( theUpdateStruct.cBackButton && m_outButton2.isConnected() )
			    m_outButton2.send( Measurement::Button( timestamp, Math::Scalar < int > ( ' ' ) ) );
			  
			  break;
			case CMM_TIMEOUT_DETECT:
			  LOG4CPP_WARN( logger, "FaroArm: Timeout occurred in communication with arm. Abort\n" );
			  break;
			case CMM_ERROR_DETECT:
			  LOG4CPP_ERROR( logger, "FaroArm: Error reported by arm: %i\n" );
			  break;
			case CMM_INACCURATE_DATA_DETECT:
			  LOG4CPP_WARN( logger, "FaroArm: Inaccurate data detected by arm. Not sending" );
			  break;
			}
			
			theUpdateStruct.cUpdateFlag = 0;

			// covert frequency given in Hz into ms
			//Util::sleep( ( int ) ( 1000.0 / m_frequency ) );
		}
		

		close( hArmLib, pCmmSpecific );
		LOG4CPP_INFO (logger, "FaroArm disconnected" );
	}
	
	Math::Quaternion FaroArmDriver::euler2Quat( double heading, double attitude, double bank )
	{
		Math::Quaternion q;
		
		boost::math::quaternion< double >       bq1( cos( bank/2.), 0, 0, sin( bank/2.) );
		boost::math::quaternion< double >       bq2( cos( attitude/2.), sin( attitude/2.) ,0 ,0 );
		boost::math::quaternion< double >       bq3( cos( heading/2.), 0, 0, sin( heading/2.) );
		boost::math::quaternion< double >       bq4;
	
		bq4 = bq3 * bq2 * bq1;	
		
		q = Math::Quaternion( bq4.R_component_2(),
			bq4.R_component_3(),
			bq4.R_component_4(),
			bq4.R_component_1()
			);
		return q;
	}
	
	void FaroArmDriver::sendPose( const Measurement::Timestamp timestamp, double rawFaroData[] )
	{	
		// Convert euler angles (in degree) to quaternion
		Math::Quaternion q = euler2Quat( rawFaroData[3] * M_PI/180 , rawFaroData[4] * M_PI/180, rawFaroData[5] * M_PI/180 );
		//Converting mm to m
		Math::Vector3d  pos ( rawFaroData[0] / 1000, rawFaroData[1] / 1000, rawFaroData[2] / 1000 );
		Math::Pose pose ( q, pos);
		if (pose != m_last_pose) {
			Measurement::Pose MeasPose( timestamp, pose );
			m_outPort.send( MeasPose);
			m_last_pose = pose;
		} else {
			LOG4CPP_WARN( logger, "FaroArm: Duplicate pose detected, skipping ..." );
		}
	}
	
	void FaroArmDriver::close( HMODULE hArmLib, int ( *pCmmSpecific )( void * ) )
	{
		int nClose = DLL_CLOSE_CMM;
		pCmmSpecific( ( void * ) &nClose );

		FreeLibrary( hArmLib );
	}
	
	void FaroArmDriver::stop()
	{
		if ( m_running )
		{
			m_running = false;
			m_bStop = true;
			if ( m_pThread )
			{
				m_pThread->join();
			}
		}
	}
	
	void FaroArmDriver::start()
	{
		if ( !m_running )
		{
			m_running = true;
			m_bStop = false;
			m_pThread.reset( new boost::thread( boost::bind( &FaroArmDriver::threadMethod, this ) ) );
		}
	}

	void FaroArmDriver::receiveLatency( const Measurement::Distance& m ) {
		double l = *m;
		LOG4CPP_DEBUG( logger , "FaroArmDriver received new latency measurement in ms: " << l );
		// convert ms to timestamp offset
		m_latency = (long int)(1000000.0 * l);
	};


UBITRACK_REGISTER_COMPONENT( Ubitrack::Dataflow::ComponentFactory* const cf ) {
	cf->registerComponent< Ubitrack::Driver::FaroArmDriver > ( "FaroArm" );
}

} } // namespace Ubitrack::Driver
