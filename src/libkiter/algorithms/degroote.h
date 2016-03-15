/*
 * deGroote.h
 *
 *  Created on: 5 juil. 2013
 *      Author: toky
 */

#ifndef DEGROOTE_H_
#define DEGROOTE_H_

namespace algorithms {


models::EventGraph*     generate_LCG   		                  (models::Dataflow* const  dataflow);
void                    clean_LCG   		                  (models::EventGraph*      lcg);
    void				compute_deGroote_throughput   		  (models::Dataflow* const  dataflow, parameters_list_t);
    void				compute_deGrooteClean_throughput	  (models::Dataflow* const  dataflow, parameters_list_t);

}

#endif /* DEGROOTE_H_ */
