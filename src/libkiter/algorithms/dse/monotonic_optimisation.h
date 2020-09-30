/*
 * monotonic_optimisation.h
 *
 *  Created on: Sep 28, 2020
 *      Author: jkmingwen
 */

#ifndef SRC_LIBKITER_ALGORITHMS_DSE_MONOTONIC_OPTIMISATION_H_
#define SRC_LIBKITER_ALGORITHMS_DSE_MONOTONIC_OPTIMISATION_H_

#include <commons/KiterRegistry.h>
#include <algorithms/schedulings.h>
#include <algorithms/throughput/kperiodic.h>

namespace models {
	class Dataflow;
}

namespace algorithms {
        /* kperiodic_result_t compute_Kperiodic_throughput_and_cycles(models::Dataflow* const dataflow, parameters_list_t params); */
  void update_infeasible_set(StorageDistributionSet infeasible_sd,
                             StorageDistribution new_sd);
  void monotonic_optimised_Kperiodic_throughput_dse(models::Dataflow* const dataflow, parameters_list_t params);
        
}

ADD_TRANSFORMATION(MonotonicOptimisation,
		transformation_t({"MonotonicOptimisation", "Use monotonic optimisation to reduce search space of buffer sizing and throughput exploration, using K-periodic scheduling method", algorithms::monotonic_optimised_Kperiodic_throughput_dse}));
#endif /* SRC_LIBKITER_ALGORITHMS_DSE_MONOTONIC_OPTIMISATION_H_ */
