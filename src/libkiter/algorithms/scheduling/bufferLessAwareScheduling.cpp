/*
 * bufferLessAwareScheduling.cpp
 *
 *  Created on: Aug 22, 2019
 *      Author: toky
 */


#include <algorithms/repetition_vector.h>
#include <algorithms/schedulings.h>
#include <commons/glpsol.h>

void bufferless_scheduling (models::Dataflow* const  dataflow, std::map<Vertex,EXEC_COUNT> &  kvector, std::vector<std::vector <Vertex> > task_sequences) {


    commons::ValueKind CONTINUE_OR_INTEGER = commons::KIND_CONTINUE;
   // With gurobi might be needed, need to fix that.
    //CONTINUE_OR_INTEGER = commons::KIND_INTEGER;

    VERBOSE_ASSERT(dataflow,TXT_NEVER_HAPPEND);

    {ForEachVertex(dataflow,t) {
        std::string name = dataflow->getVertexName(t);
        VERBOSE_INFO(" - " << name << " k=" <<  kvector[t]);
        VERBOSE_ASSERT(kvector[t], "periodicity vector must be positive non zero.");
    }}
    VERBOSE_INFO("Getting period ...");

    // STEP 0 - CSDF Graph should be normalized
    VERBOSE_ASSERT(computeRepetitionVector(dataflow),"inconsistent graph");

    VERBOSE_INFO("Generate Graph ...");

    //##################################################################
    // Linear program generation
    //##################################################################
    const std::string problemName =  "BLKPeriodicSizing_" + dataflow->getName() + "_" + ((CONTINUE_OR_INTEGER == commons::KIND_INTEGER) ? "INT" : "");
    commons::GLPSol g = commons::GLPSol(problemName,commons::MIN_OBJ);

    // Starting times
    //******************************************************************
    {ForEachVertex(dataflow,t) {
        std::string name = dataflow->getVertexName(t);
        for(EXEC_COUNT k = 1; k <= kvector[t] ; k++) {
            g.addColumn("s_" + commons::toString<EXEC_COUNT>(k) + "_" + name,commons::KIND_CONTINUE,commons::bound_s(commons::LOW_BOUND,0),0);
        }
    }}


    for (std::vector<Vertex> sequence : task_sequences) {
        VERBOSE_INFO("One sequence of " << sequence.size() << " task to force inline");
        if (sequence.size() == 0) continue;
        EXEC_COUNT force_k = kvector[sequence[0]];

        std::vector<std::string> previous;
        std::string name = dataflow->getVertexName(sequence[0]);

        for(EXEC_COUNT k = 1; k <= kvector[sequence[0]] ; k++) {
        	previous.push_back("s_" + commons::toString<EXEC_COUNT>(k) + "_" + name);
        }

        for (Vertex t : sequence) {
        	VERBOSE_ASSERT(force_k == kvector[t], "Edges from the same sequence must have the same k values.");

            for(EXEC_COUNT k = 1; k <= kvector[t] ; k++) {

            	// ADD sequence between between current task k and previous task k.
            	std::string previous_name = previous[k-1];
            	std::string current_name = "s_" + commons::toString<EXEC_COUNT>(k) + "_" + dataflow->getVertexName(t);

            	if (previous_name != current_name)  {
            		// add constraint
            		std::string row_name = "sequence_" + previous_name + "_to_" + current_name;
            		g.addRow(row_name,commons::bound_s(commons::FIX_BOUND, 1 ));
            		g.addCoef(row_name , previous_name   , - 1     );
            		g.addCoef(row_name , current_name    ,   1     );
                    VERBOSE_INFO("Add " << row_name);
            	}
            	previous[k-1] = current_name;
            }

        }


    }

    auto OMEGA_COL = g.addColumn("OMEGA",commons::KIND_CONTINUE,commons::bound_s(commons::LOW_BOUND,0),1);





    // Constraints
    //******************************************************************

    {ForEachEdge(dataflow,c) {
        const Vertex source   = dataflow->getEdgeSource(c);
        const Vertex target   = dataflow->getEdgeTarget(c);

        const std::string  buffername= dataflow->getEdgeName(c);
        const std::string  sourceStr = dataflow->getVertexName(source);
        const std::string  targetStr = dataflow->getVertexName(target);
        const EXEC_COUNT  Ni        =  dataflow->getNi(source);

        const TOKEN_UNIT  in_b        = dataflow->getEdgeIn(c);
        const TOKEN_UNIT  ou_b        = dataflow->getEdgeOut(c);

        const TOKEN_UNIT  gcdb      = boost::math::gcd((in_b),(ou_b));
        const TOKEN_UNIT  gcdk      = boost::math::gcd( kvector[source]  * (in_b), kvector[target] * (ou_b));

        const TOKEN_UNIT  mop      =  commons::floor(dataflow->getPreload(c),gcdb);



        const TIME_UNIT       ltai    = dataflow->getVertexDuration(source,1);
        const TOKEN_UNIT  Ha        =   std::max((TOKEN_UNIT)0, in_b - ou_b);

        for(EXEC_COUNT ai = 1; ai <= kvector[source] ; ai++) {
            int saicolid = g.getColumn("s_" + commons::toString<EXEC_COUNT>(ai) + "_"+ sourceStr );
            for(EXEC_COUNT  aj = 1; aj <= kvector[target] ; aj++) {
                int sajcolid = g.getColumn("s_" + commons::toString<EXEC_COUNT>(aj) + "_"+ targetStr );


                // *** Normal Buffer constraint computation
                const TOKEN_UNIT  alphamin  =   commons::ceil(Ha + (TOKEN_UNIT) aj * ou_b - (TOKEN_UNIT) ai * in_b - mop,gcdk);
                const TOKEN_UNIT  alphamax  =   commons::floor(  in_b + (TOKEN_UNIT)aj * ou_b - (TOKEN_UNIT)ai * in_b  - mop - 1 ,gcdk);


                if (alphamin <= alphamax) { // check if contraint exist
                    const std::string pred_row_name = "precedence_" + buffername + "_" + commons::toString<EXEC_COUNT>(ai) + "_" + commons::toString<EXEC_COUNT>(aj);
                    TIME_UNIT coef =  ((((TIME_UNIT) alphamax) / ( (TIME_UNIT) Ni  * (TIME_UNIT) in_b )));



                    int rowid = g.addRow(pred_row_name,commons::bound_s(commons::LOW_BOUND, (double) ltai ));
                    VERBOSE_INFO("Add " << pred_row_name);



                    g.fastAddCoef(rowid ,OMEGA_COL    , (double) - coef      );

                    if ( (ai != aj) || (source != target)) {
                        g.fastAddCoef(rowid ,sajcolid    ,  1        );
                        g.fastAddCoef(rowid ,saicolid    , -1        );
                    }
                }
            }
        }

    }}




    //##################################################################
    // SOLVE LP
    //##################################################################

    // commons::GLPParameters ilp_params = commons::getDefaultParams();

    // ilp_params.general_doScale = true;
    // ilp_params.linear_doAdvBasis = true;
    // ilp_params.linear_method = commons::DUAL_LINEAR_METHOD;
    //
    // bool sol = g.solve(ilp_params);


    VERBOSE_INFO("Solving problem ...");

    bool sol = g.solve();

    VERBOSE_INFO("Solved, gathering results ...");

    //##################################################################
    // GATHERING RESULTS
    //##################################################################

    // BUFFER SIZES
    //******************************************************************
    if (sol) {

    	TIME_UNIT OMEGA = g.getValue("OMEGA");

        std::cout << "OMEGA : " << OMEGA << std::endl ;

        {ForEachVertex(dataflow,t) {
            std::string name = dataflow->getVertexName(t);
            for(EXEC_COUNT k = 1; k <= kvector[t] ; k++) {
                auto starting_time = g.getValue("s_" + commons::toString<EXEC_COUNT>(k) + "_" + name);

                std::cout << "s_" << k  << "_" << name <<  "=" << starting_time
                		<< "  NI=" <<  dataflow->getNi(t)
        	         	<< "  period=" <<  OMEGA / (TIME_UNIT) ( (TIME_UNIT) dataflow->getNi(t) / (TIME_UNIT) kvector[t] ) << std::endl ;
            }
        }}


    } else {
        VERBOSE_ERROR("No feasible solution");
    }

    VERBOSE_INFO("Done");
    return;


}



void algorithms::scheduling::KPeriodic_scheduling_bufferless (models::Dataflow* const  dataflow,  parameters_list_t   param_list) {

	 std::map<Vertex,EXEC_COUNT> kvector;
		    {ForEachVertex(dataflow,v) {
		        kvector[v] = 1;
		        if (param_list.count(dataflow->getVertexName(v)) == 1) {
		            std::string str_value = param_list[dataflow->getVertexName(v)];
		            kvector[v] =  commons::fromString<EXEC_COUNT> ( str_value );
		        }
	}}

	bufferless_scheduling(dataflow,kvector, {});
}

