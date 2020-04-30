/*
 * kperiodic.h
 *
 *  Created on: 9 mai 2013
 *      Author: toky
 */

#ifndef KPERIODIC_H_
#define KPERIODIC_H_

#include <algorithms/schedulings.h>
#include <commons/KiterRegistry.h>


#define VERBOSE_KPERIODIC_DEBUG(msg) {VERBOSE_CUSTOM_DEBUG("keperiodic",msg);}


namespace models {
	class Dataflow;
	class EventGraph;
}


namespace algorithms {

bool sameset(models::Dataflow* const dataflow, std::set<Edge> *cc1 , std::set<Edge>* cc2) ;
std::string cc2string  (models::Dataflow* const dataflow,std::set<Edge>* cc) ;
models::EventGraph* generateKPeriodicEventGraph(models::Dataflow * const dataflow, std::map<Vertex,EXEC_COUNT> * kvector  );

std::string print_schedule (models::EventGraph* eg, models::Dataflow* const  dataflow,  std::map<Vertex,EXEC_COUNT> & kvector , TIME_UNIT res ) ;

scheduling_t generateKperiodicSchedule    (models::Dataflow* const dataflow) ;
    void print_kperiodic_scheduling         (models::Dataflow* const  dataflow, parameters_list_t);
    void print_kperiodic_expansion_graph    (models::Dataflow* const  dataflow, parameters_list_t);
  void generateKperiodicSelfloop(models::Dataflow * const dataflow , EXEC_COUNT ki, models::EventGraph* g  , Vertex t  );

  void generateKPeriodicConstraint(models::Dataflow * const dataflow , std::map<Vertex,EXEC_COUNT> * kValues,  models::EventGraph* g, Edge c) ;

  void fastGenerateKPeriodicConstraint(models::Dataflow * const dataflow , std::map<Vertex,EXEC_COUNT> * kValues,  models::EventGraph* g, Edge c) ;
  void newGenerateKPeriodicConstraint(models::Dataflow * const dataflow , std::map<Vertex,EXEC_COUNT> * kValues,  models::EventGraph* g, Edge c) ;

  models::EventGraph*  updateEventGraph( models::Dataflow * const dataflow , std::map<Vertex,EXEC_COUNT> * kvector, std::set<Edge>* cc, models::EventGraph* g) ;

models::EventGraph* generateKPeriodicEventGraphWithNormalize  (models::Dataflow *  const,  std::map<Vertex,EXEC_COUNT> * kvector, bool noRe  );


 void updateKPeriodicEventGraph(models::Dataflow * const dataflow , std::map<Vertex,EXEC_COUNT> * kValues,   std::set<Edge> * cc, models::EventGraph* g  );
 models::EventGraph*  generateCycleOnly(models::Dataflow * const dataflow , std::map<Vertex,EXEC_COUNT> * kValues,   std::set<Edge> * cc );

 std::map<Vertex,EXEC_COUNT>  get_Kvector(models::Dataflow *  const);
void                compute_Kperiodic_throughput              (models::Dataflow *  const , parameters_list_t);

    void                compute_1Kperiodic_throughput   		  (models::Dataflow* const  dataflow, parameters_list_t);
    void                compute_2Kperiodic_throughput             (models::Dataflow* const  dataflow, parameters_list_t);
    void                compute_NKperiodic_throughput             (models::Dataflow* const  dataflow, parameters_list_t);
    void                compute_GKperiodic_throughput  		      (models::Dataflow* const  dataflow, parameters_list_t);

    kperiodic_result_t KSchedule             (models::Dataflow *  const ,std::map<Vertex,EXEC_COUNT> * kvector  , TIME_UNIT bound = 0) ;
    kperiodic_result_t KScheduleBufferLess   (models::Dataflow *  const ,std::map<Vertex,EXEC_COUNT> * kvector  , TIME_UNIT bound = 0) ;

    void                                     updateVectorWithFullNi   (models::Dataflow *  const  ,std::map<Vertex,EXEC_COUNT> *  , std::set<Edge> * ) ;
    bool                                     updateVectorWithLocalNi  (models::Dataflow *  const  ,std::map<Vertex,EXEC_COUNT> *  , std::set<Edge> * ) ;
    bool 									 updateVectorWithFineNi   (models::Dataflow * const   , std::map<Vertex,EXEC_COUNT> * , std::set<Edge>*  );

    TIME_UNIT compute_Kperiodic_optimal_throughput    (models::Dataflow* const dataflow) ;

    
    void compute_KperiodicSlow_throughput    (models::Dataflow* const dataflow, parameters_list_t) ;
    void compute_KperiodicSlow2_throughput    (models::Dataflow* const dataflow, parameters_list_t) ;

    void print_1periodic_eventgraph                                 (models::Dataflow* const  dataflow, parameters_list_t);
    void print_2periodic_eventgraph                                 (models::Dataflow* const  dataflow, parameters_list_t);
    void print_NKperiodic_eventgraph                                (models::Dataflow *  const , parameters_list_t);
    EXEC_COUNT test_Kperiodic_throughput    (models::Dataflow* const dataflow, parameters_list_t) ;
}
ADD_TRANSFORMATION(KPeriodicThroughput,
		transformation_t({ "KPeriodicThroughput" , "Optimal Throughput evaluation of CSDF by K-Periodic scheduling method 2.", algorithms::compute_Kperiodic_throughput}));

ADD_TRANSFORMATION(UnPeriodicThroughput,
		transformation_t({ "1PeriodicThroughput" , "Optimal 1-Periodic Throughput evaluation of CSDF by K-Periodic scheduling method.", algorithms::compute_1Kperiodic_throughput}));
ADD_TRANSFORMATION(DeuxPeriodicThroughput,
				transformation_t({ "2PeriodicThroughput" , "Optimal 1-Periodic Throughput evaluation of CSDF by K-Periodic scheduling method.", algorithms::compute_2Kperiodic_throughput}));
ADD_TRANSFORMATION(NKPeriodicThroughput,
		transformation_t({ "NKPeriodicThroughput" , "Optimal Throughput evaluation of CSDF by using N-periodic method.", algorithms::compute_NKperiodic_throughput}));


ADD_TRANSFORMATION(PrintKPeriodicScheduling,
		transformation_t({ "PrintKPeriodicScheduling" , "Print KPeriodicScheduling", algorithms::print_kperiodic_scheduling})
);

#endif /* KPERIODIC_H_ */
