/*
 * schedulings.h
 *
 *  Created on: Aug 23, 2019
 *      Author: toky
 */

#ifndef SRC_LIBKITER_ALGORITHMS_SCHEDULINGS_H_
#define SRC_LIBKITER_ALGORITHMS_SCHEDULINGS_H_

#include <map>
#include <commons/KiterRegistry.h>
#include <models/Dataflow.h>
#include <models/Scheduling.h>


typedef std::map<Vertex,EXEC_COUNT> periodicity_vector_t;



scheduling_t period2scheduling    (models::Dataflow* const  dataflow,  periodicity_vector_t & kvector , TIME_UNIT throughput) ;
models::Scheduling  period2Scheduling    (models::Dataflow* const  dataflow,  periodicity_vector_t & kvector , TIME_UNIT throughput) ;

namespace algorithms {


void BufferlessNoCScheduling(models::Dataflow* const  dataflow, parameters_list_t   param_list);

	namespace scheduling {
		void KPeriodic_taskNoCbufferless(models::Dataflow*, parameters_list_t param_list);
		scheduling_t bufferless_scheduling(models::Dataflow* const  dataflow, periodicity_vector_t &  kvector);
		void sdf_bufferless_scheduling (models::Dataflow* const  dataflow, periodicity_vector_t &  kvector, std::vector<std::vector <Vertex> > task_sequences);
		scheduling_t bufferless_kperiodic_scheduling(models::Dataflow* const  dataflow, bool stop_at_first, bool get_previous);
		void bufferlessKPeriodicScheduling (models::Dataflow* const  dataflow, parameters_list_t params) ;

		models::Scheduling CSDF_KPeriodicScheduling       (models::Dataflow* const dataflow) ;
		const periodicity_vector_t generate1PeriodicVector(const models::Dataflow* dataflow);
		const periodicity_vector_t generateKPeriodicVector(const models::Dataflow* dataflow, int k);
		const periodicity_vector_t generateNPeriodicVector(const models::Dataflow* dataflow);

		 models::Scheduling CSDF_KPeriodicScheduling_LP    (const models::Dataflow* const dataflow, const periodicity_vector_t& kvector);
		 void CSDF_1PeriodicScheduling_LP (models::Dataflow*  dataflow, parameters_list_t );
		 void CSDF_NPeriodicScheduling_LP (models::Dataflow*  dataflow, parameters_list_t );
		 void CSDF_NPeriodicScheduling    (models::Dataflow*  dataflow, parameters_list_t );


	}
}


// Recent stuff
ADD_TRANSFORMATION(LP1,
transformation_t({ "LP1" , "Rewriting Bodin2016 Threshold CSDF 1-Periodic Scheduling with Bufferless channel using Linear Programming", algorithms::scheduling::CSDF_1PeriodicScheduling_LP}));
ADD_TRANSFORMATION(LPN,
transformation_t({ "LPN" , "Rewriting Bodin2016 Threshold CSDF N-Periodic Scheduling with Bufferless channel using Linear Programming", algorithms::scheduling::CSDF_NPeriodicScheduling_LP}));
ADD_TRANSFORMATION(EGN,
transformation_t({ "EGN" , "Rewriting Bodin2013 Threshold CSDF Periodic Scheduling", algorithms::scheduling::CSDF_NPeriodicScheduling}));
ADD_TRANSFORMATION(BufferlessKPeriodicScheduling,
		transformation_t({ "BufferlessKPeriodicScheduling" , "Run Bufferless Kperiodic", algorithms::scheduling::bufferlessKPeriodicScheduling} )
	);


// Throughput techniques
ADD_TRANSFORMATION(BufferlessNoCScheduling,
		transformation_t({ "BufferlessNoCScheduling" , "BufferlessNoCScheduling, WIP", algorithms::BufferlessNoCScheduling}));








#endif /* SRC_LIBKITER_ALGORITHMS_SCHEDULINGS_H_ */
