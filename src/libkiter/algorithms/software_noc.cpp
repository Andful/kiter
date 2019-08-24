/*
 * kperiodic.cpp
 *
 *  Created on: 9 mai 2013
 *      Author: toky
 */

#include <models/NoC.h>
#include <vector>
#include <iostream>
#include <printers/stdout.h>
#include <commons/verbose.h>
#include <models/Dataflow.h>
#include <models/EventGraph.h>
#include <algorithms/normalization.h>
#include <algorithms/software_noc.h>
#include <algorithms/repetition_vector.h>
#include <algorithms/kperiodic.h>
#include <cstdlib>
#include <stack>
#include <boost/math/common_factor.hpp>
// What is it for ? 
// #include <bits/stdc++.h>
#include <set>
#include <queue>

//vector stores the cycle length in each of the vertices
std::vector< std::vector<unsigned int> > cycles;
std::vector< std::set<unsigned int> > cycid_per_vtxid;
std::vector< std::vector<unsigned int> > cyclen_per_vtxid;

struct node{
   ARRAY_INDEX index;
};

bool operator<(const node& a, const node& b) {

	auto alen = cyclen_per_vtxid[a.index].size();
	auto blen = cyclen_per_vtxid[b.index].size();
	auto lenmin = std::min(alen, blen);

	for(unsigned int i = 0; i < lenmin; i++)
	{
		if(cyclen_per_vtxid[a.index][i] == cyclen_per_vtxid[b.index][i])
			continue;
		else
			return cyclen_per_vtxid[a.index][i] < cyclen_per_vtxid[b.index][i];
	} 

	if(alen == 0)
		return true;
	if(blen == 0)
		return false;

	return false;
}


LARGE_INT gcdExtended(LARGE_INT x, LARGE_INT y, LARGE_INT *a, LARGE_INT *b)
{
    // Base Case
    if (x == 0)
    {
        *a = 0;
        *b = 1;
        return y;
    }

    LARGE_INT a1, b1; // To store results of recursive call
    LARGE_INT gcd = gcdExtended(y%x, x, &a1, &b1);

    // Update x and y using results of recursive
    // call
    *a = b1 - (y/x) * a1;
    *b = a1;

    return gcd;
}


void showstack(std::stack<Vertex> s) 
{
    while (!s.empty()) 
    { 
        std::cout << '\t' << s.top(); 
        s.pop(); 
    } 
    std::cout << '\n'; 
}


void DFSUtil(Vertex v, std::vector<bool>& visited, models::Dataflow* to, std::vector<Vertex>& traversal)
{ 
	// Mark the current node as visited and print it
	auto myid = to->getVertexId(v);
	visited[myid] = true; 
	traversal.push_back(v);

	// Recur for all the vertices adjacent to this vertex 
	{ForOutputEdges(to,v,e){
		Vertex end = to->getEdgeTarget(e);
		ARRAY_INDEX endid = to->getVertexId(end);
		if(!visited[endid])
			DFSUtil(end, visited, to, traversal);
	}}
} 


void printVector(std::vector<Vertex>& traversal, models::Dataflow* to)
{
	for(unsigned int trav_i = 0; trav_i < traversal.size(); trav_i++)
		std::cout << to->getVertexId(traversal[trav_i]) << " ";
	std::cout << std::endl;
}


//find all parents and set their value with size
void updateAncestorsHelper(Vertex vtx, models::Dataflow* to, std::set<unsigned int>& myset, std::vector<bool>& visited)
{
	auto index = to->getVertexId(vtx);
	for(auto it = myset.begin(); it != myset.end(); it++)
		cycid_per_vtxid[index].insert(*it);
	visited[index] = true;

	{ForInputEdges(to,vtx,e){
		Vertex source = to->getEdgeSource(e);
		auto sourceid = to->getVertexId(source);
		if(!visited[sourceid])
			updateAncestorsHelper(source, to, myset, visited);
	}}
}


void updateAncestorsCycle(Vertex vtx, std::set<unsigned int>& myset, models::Dataflow* to)
{
	int V = to->getVerticesCount();
	std::vector<bool> visited(V+1, false);
	updateAncestorsHelper(vtx, to, myset, visited);
}


// DFS traversal of the vertices reachable from v. It uses recursive DFSUtil() 
void DFS(models::Dataflow* to, Vertex v) 
{
	int V = to->getVerticesCount(); 
	// Mark all the vertices as not visited 
	std::vector<bool> visited(V+1, false); 
	std::cout << "DFS Order: ";

	// Call the recursive helper function to print DFS traversal 
	std::vector<Vertex> traversal;
	DFSUtil(v, visited, to, traversal);

	printVector(traversal, to);
}


void my_dfs(models::Dataflow* adj, Vertex node, std::vector<bool> visited, Vertex start, std::vector<unsigned int>& path, std::set<unsigned int>& cycleids)
{
	auto node_id = adj->getVertexId(node);
	if (visited[node_id])
	{
		if (node == start)
		{
			//std::cout << "found a path, len=" << path.size() << "\n";
			std::vector<unsigned int> temp = path;
			sort(temp.begin(), temp.end());
			bool found = false;
			for(unsigned int i = 0; i < cycles.size(); i++)
			{
				if(cycles[i] == temp)
				{
					found = true;
					cycleids.insert((unsigned int)i);
					break;
				}
			}
			if(!found)
			{
				cycles.push_back(temp);
				cycleids.insert((unsigned int)cycles.size()-1);
				//std::cout << "adding a cycle\n";
			}
		}
		return;
	}

	visited[node_id]=true;
	path.push_back((unsigned int)node_id);

	{ForOutputEdges(adj,node,e) {
		auto child = adj->getEdgeTarget(e);
		my_dfs(adj, child, visited, start, path, cycleids);
	}}

	visited[node_id]=false;
	while(path.back() != node_id)
		path.pop_back();
	//popping the node_id element as well
	path.pop_back();
}


std::set<unsigned int> my_dfs_wrapper(models::Dataflow* adj, Vertex start)
{	
	int V = adj->getVerticesCount();
	std::vector<bool> visited(V+1, false);

	std::vector<unsigned int> path;

	//Call the above function with the start node:
	std::set<unsigned int> mycycleids;
	my_dfs(adj,start,visited, start, path, mycycleids);

	return mycycleids;
}


models::Dataflow getTranspose(models::Dataflow* to) 
{
	//int V = to->getVerticesCount();
	//std::cout << "vertices count: " << V << "\n";

	models::Dataflow out;
	std::map<ARRAY_INDEX, Vertex> out_vtx_map;

	{ForEachVertex(to,source) {
		auto s_id = to->getVertexId(source);
		out_vtx_map[s_id] = out.addVertex();
	}}

	// Note: getEdgeOut and getEdgeIn are Output and input Rates of a buffer
	{ForEachVertex(to,source) {
		{ForOutputEdges(to,source,e) {
			auto target = to->getEdgeTarget(e);
			auto s_id = to->getVertexId(source);
			auto t_id = to->getVertexId(target);
			//std::cout << "out: " << t_id << "," << s_id << "\n";
			out.addEdge(out_vtx_map[t_id], out_vtx_map[s_id]);
			//std::cout << "end: \n";
		}}
	}}

	return out;
} 


void fillOrder(Vertex vtx, std::vector<bool>& visited, std::stack<Vertex> &Stack, models::Dataflow* to) 
{ 
	auto v = to->getVertexId(vtx);
	// Mark the current node as visited and print it 
	visited[v] = true; 

	//std::cout << "v=" << v << "\n";
	// Recur for all the vertices adjacent to this vertex 
        {ForOutputEdges(to,vtx,e){
                auto end = to->getEdgeTarget(e);
                auto endid = to->getVertexId(end);
                if(!visited[endid])
			fillOrder(end, visited, Stack, to);
        }}

	// All vertices reachable from v are processed by now, push v  
	Stack.push(vtx); 
} 

 
// The main function that finds and prints all strongly connected components 
void printSCCs(models::Dataflow* to, Vertex start)
{
	int V = to->getVerticesCount(); 
	std::stack<Vertex> Stack; 

	// Mark all the vertices as not visited (For first DFS) 
	std::vector<bool> visited(V+1, false); 
 
	//initiate with start node first
	fillOrder(start, visited, Stack, to);
	// Fill vertices in stack according to their finishing times 
        {ForEachVertex(to,t) {
		auto v = to->getVertexId(t);
		if(!visited[v])
			fillOrder(t, visited, Stack, to);
        }}

	// Create a reversed graph 
	auto transp = getTranspose(to); 
	// Mark all the vertices as not visited (For second DFS) 
	for(int i = 0; i <= V; i++) 
		visited[i] = false; 

	// Now process all vertices in order defined by Stack 
	while (Stack.size() != 0)
	{
		// Pop a vertex from stack 
		Vertex vtx = Stack.top();
		auto v = to->getVertexId(vtx); 
		Stack.pop(); 

		// Print Strongly connected component of the popped vertex 
		if (visited[v] == false) 
		{
			std::vector<Vertex> traversal;
			DFSUtil(vtx, visited, &transp, traversal);
			printVector(traversal, to);

			if(traversal.size() > 1)
			{
				for(unsigned int i = 0; i < traversal.size(); i++)
				{
					auto temp_vtx = traversal[i];
					std::set<unsigned int> mycycles = my_dfs_wrapper(to, temp_vtx);
					updateAncestorsCycle(temp_vtx, mycycles, to);
				}
			}
		}
	}

	for(int i = 0; i <= V; i++)
	{
		std::vector<unsigned int> temp;
		cyclen_per_vtxid.push_back(temp);

		if(cycid_per_vtxid[i].size() > 0)
			std::cout << "vtx=" << i << ":";
		for(auto it = cycid_per_vtxid[i].begin(); it != cycid_per_vtxid[i].end(); it++)
		{
			std::cout << cycles[*it].size() << " ";
			cyclen_per_vtxid[i].push_back( (unsigned int)cycles[*it].size() );
		}
		if(cycid_per_vtxid[i].size() > 0)
		{
			std::cout << "\n";
			sort(cyclen_per_vtxid[i].begin(), cyclen_per_vtxid[i].end());
		}
	}
}


//Process the graph for DFS, etc. in this function
Vertex graphProcessing(models::Dataflow* to)
{
	bool myflag = false;
	bool myflag2 = false;
	Vertex top;

	auto start = to->addVertex();
	to->setVertexName(start, "start");
	{ForEachVertex(to,t) {
		if( (t != start) && (!myflag) && (!myflag2))
		{
			top = t;
			myflag2 = true;
		}

		if(t == start)
		{
			continue;
		}
		else if(to->getVertexInDegree(t) == 0)
		{
			myflag = true;
			to->addEdge(start, t);
		}
	}}

	if(!myflag)
		to->addEdge(start, top);

	int V = to->getVerticesCount();
	for(int i = 0; i < V+1; i++)
	{
		std::set<unsigned int> temp;
		cycid_per_vtxid.push_back(temp);
	}

	printSCCs(to, start);
	return start;
}



//remove the current edge between nodes add intermediate nodes based on the path between them
void taskAndNoCMapping(models::Dataflow* to, Vertex start, NoC* noc)//, Edge c, NoC* noc,  std::map< unsigned int, std::vector< std::pair<Vertex, Vertex> > > & returnValue)
{
	int V = to->getVerticesCount();
	std::priority_queue<node> pq;
	std::vector<bool> visited(V+1, false);

	auto top = start;
	ARRAY_INDEX endid = to->getVertexId(top);
	node temp2;
	temp2.index = endid;
	visited[endid] = true;
	pq.push(temp2);
	std::cout << "adding " << top << "\n";

	while(!pq.empty())
	{
		top = to->getVertexById(pq.top().index);
		pq.pop();

		if(top != start)
			std::cout << "mapping " << to->getVertexId(top) << "\n";
		{ForOutputEdges(to, top, e){
			Vertex end = to->getEdgeTarget(e);
			ARRAY_INDEX endid = to->getVertexId(end);
			if(!visited[endid])
			{
				node temp;
				temp.index = endid;
				visited[endid] = true;
				pq.push(temp);
			}
		}}
	}


	/*
	// We store infos about edge to be deleted
	auto source_vtx = d->getEdgeSource(c);
	auto target_vtx = d->getEdgeTarget(c);

	//Find the core index
	auto source = d->getVertexId(source_vtx)-1; //minus 1 since the nodes start from 1 instead of zero
	auto target = d->getVertexId(target_vtx)-1;

	//Core mapping, modulo scheduling
	source = source % noc->getMeshSize();
	target = target % noc->getMeshSize();

	//use the inrate and route of the edges ans use it when creating the edges
	auto inrate = d->getEdgeIn(c);
	auto outrate = d->getEdgeOut(c);
	auto preload = d->getPreload(c);  // preload is M0

	bool flag = true;
	if (source == target) //ignore this case
		return;

	// we delete the edge
	d->removeEdge(c);

	//for every link in the path, add a corresponding node
	auto list = noc->get_route(source, target);
	for (auto e : list) {
		//std::cout << e << " --> " ;
		// we create a new vertex "middle"
		auto middle = d->addVertex();

		std::stringstream ss;
		ss << "mid-" << source << "," << target << "-" << e;

		std::pair<Vertex, Vertex> pair_temp;
		pair_temp.first = middle;
		pair_temp.second = source_vtx;

		returnValue[(unsigned int)e].push_back(pair_temp);
		d->setVertexName(middle,ss.str());

		d->setPhasesQuantity(middle,1); // number of state for the actor, only one in SDF
		d->setVertexDuration(middle,{1}); // is specify for every state , only one for SDF.
		d->setReentrancyFactor(middle,1); // This is the reentrancy, it avoid a task to be executed more than once at the same time.

		// we create a new edge between source and middle,
		auto e1 = d->addEdge(source_vtx, middle);

		if(flag)
		{
			d->setEdgeInPhases(e1,{inrate});  // we specify the production rates for the buffer
			flag = false;
		}
		else
		{
			d->setEdgeInPhases(e1,{1});
		}

		d->setEdgeOutPhases(e1,{1}); // and the consumption rate (as many rates as states for the associated task)
		d->setPreload(e1,preload);  // preload is M0

		source_vtx = middle;
	}

	//find the final edge
	auto e2 = d->addEdge(source_vtx, target_vtx);
	d->setEdgeOutPhases(e2,{outrate});
	d->setEdgeInPhases(e2,{1});
	d->setPreload(e2,0);  // preload is M0

	*/
}
















//remove the current edge between nodes
//add intermediate nodes based on the path between them
void addPathNode(models::Dataflow* d, Edge c, NoC* noc,  std::map< unsigned int, std::vector< std::pair<Vertex, Vertex> > > & returnValue)
{
	// We store infos about edge to be deleted
	auto source_vtx = d->getEdgeSource(c);
	auto target_vtx = d->getEdgeTarget(c);

	//Find the core index
	auto source = d->getVertexId(source_vtx)-1; //minus 1 since the nodes start from 1 instead of zero
	auto target = d->getVertexId(target_vtx)-1;

	//Core mapping, modulo scheduling
	source = source % noc->getMeshSize();
	target = target % noc->getMeshSize();

	//use the inrate and route of the edges ans use it when creating the edges
	auto inrate = d->getEdgeIn(c);
	auto outrate = d->getEdgeOut(c);
	auto preload = d->getPreload(c);  // preload is M0

	bool flag = true;
	if (source == target) //ignore this case
		return;

	// we delete the edge
	d->removeEdge(c);

	//for every link in the path, add a corresponding node
	auto list = noc->get_route(source, target);
	for (auto e : list) {
		//std::cout << e << " --> " ;
		// we create a new vertex "middle"
		auto middle = d->addVertex();

		std::stringstream ss;
		ss << "mid-" << source << "," << target << "-" << e;

		std::pair<Vertex, Vertex> pair_temp;
		pair_temp.first = middle;
		pair_temp.second = source_vtx;

		returnValue[(unsigned int)e].push_back(pair_temp);
		d->setVertexName(middle,ss.str());

		d->setPhasesQuantity(middle,1); // number of state for the actor, only one in SDF
		d->setVertexDuration(middle,{1}); // is specify for every state , only one for SDF.
		d->setReentrancyFactor(middle,1); // This is the reentrancy, it avoid a task to be executed more than once at the same time.

		// we create a new edge between source and middle,
		auto e1 = d->addEdge(source_vtx, middle);

		if(flag)
		{
			d->setEdgeInPhases(e1,{inrate});  // we specify the production rates for the buffer
			flag = false;
		}
		else
		{
			d->setEdgeInPhases(e1,{1});
		}

		d->setEdgeOutPhases(e1,{1}); // and the consumption rate (as many rates as states for the associated task)
		d->setPreload(e1,preload);  // preload is M0

		source_vtx = middle;
	}

	//find the final edge
	auto e2 = d->addEdge(source_vtx, target_vtx);
	d->setEdgeOutPhases(e2,{outrate});
	d->setEdgeInPhases(e2,{1});
	d->setPreload(e2,0);  // preload is M0
}


void addDependency(models::Dataflow* d, const Vertex vi, const Vertex vj, EXEC_COUNT ni, EXEC_COUNT nj)
{
	LARGE_INT a, b;
	auto gcd_value = gcdExtended((LARGE_INT) ni, (LARGE_INT) nj, &a, &b);

	ni /= gcd_value;
	nj /= gcd_value;
	if (d->is_read_only()) {
		d->set_writable();
	}
	auto e1 = d->addEdge(vi, vj);
	auto e2 = d->addEdge(vj, vi);

	gcd_value = ni + nj - gcdExtended((LARGE_INT) ni, (LARGE_INT) nj, &a, &b);

	d->setEdgeOutPhases(e1,{(TOKEN_UNIT)ni});
	d->setEdgeInPhases(e1,{(TOKEN_UNIT)nj});
	d->setPreload(e1,gcd_value);  // preload is M0

	d->setEdgeOutPhases(e2,{(TOKEN_UNIT)nj});
	d->setEdgeInPhases(e2,{(TOKEN_UNIT)ni});
	d->setPreload(e2,0);  // preload is M0

	std::cout << "Win=" << ni << " Wout=" << nj << " Mo=" << gcd_value << std::endl;

}


void algorithms::software_noc(models::Dataflow* const  dataflow, parameters_list_t param_list)
{
	std::map< unsigned int, std::vector< std::pair<Vertex, Vertex> > > conflictEdges; //stores details of flows that share noc edges
	int total_conflict = 0;

	//Init NoC
    	NoC *noc = new NoC(4, 4, 1);
	noc->generateShortestPaths();

	// STEP 0.2 - Assert SDF
	models::Dataflow* to = new models::Dataflow(*dataflow);
	models::Dataflow* grProc = new models::Dataflow(*dataflow); //variable used for intermediate graph processing

	//stub start srjkvr
	Vertex start = graphProcessing(grProc);
	taskAndNoCMapping(grProc, start, noc);

	exit(0);
	//stub end srjkvr

	double xscale  = 1;
	double yscale = 1;
	if (param_list.count("xscale") == 1) xscale = std::stod(param_list["xscale"]);
	if (param_list.count("yscale") == 1) yscale = std::stod(param_list["yscale"]);

	VERBOSE_ASSERT(dataflow,TXT_NEVER_HAPPEND);

	//Original graph
	std::string inputdot = printers::GenerateDOT (dataflow);
	std::ofstream outfile;
	outfile.open("input.dot");
	outfile << inputdot;
	outfile.close();

	std::cerr << " ================ " <<  to->getName() <<  " ===================== EDGE CONTENT" << std::endl;
	//Store the current edges list first
	std::vector<Edge> edges_list;
	{ForEachEdge(to,e) {
		std::cerr << to->getVertexId(to->getEdgeSource(e)) << "->" << to->getVertexId(to->getEdgeTarget(e)) << ":name:" << to->getEdgeName(e) << "[" << to->getEdgeIn(e) << "]" << "[" << to->getEdgeOut(e) << "]" << to->getEdgeId(e) << std::endl;
		edges_list.push_back(e);
	}}

 	//add intermediate nodes
	for(auto e: edges_list) {
		addPathNode(to, e, noc,conflictEdges);
	}

	std::string outputdot = printers::GenerateDOT (to);

	outfile.open("output.dot");
	outfile << outputdot;
	outfile.close();

	std::cerr << " ================ " <<  to->getName() <<  " ===================== " << std::endl;

	// Note: getEdgeOut and getEdgeIn are Output and input Rates of a buffer
	{ForEachVertex(to,t) {
		std::cerr << " vertex:" << to->getVertexName(t) << ":" << to->getVertexId(t) << std::endl;
		{ForInputEdges(to,t,e){						    
			std::cerr << " in:" << to->getEdgeName(e) << "[" << to->getEdgeOut(e) << "]"  << to->getEdgeId(e) << std::endl;
		}}
		{ForOutputEdges(to,t,e) {
			std::cerr << " out:" << to->getEdgeName(e)  << "[" << to->getEdgeIn(e) << "]"  << to->getEdgeId(e) << std::endl;
		}}
	}}

	VERBOSE_ASSERT(computeRepetitionVector(to),"inconsistent graph");
	// To fix: RUN 

	std::map<Vertex,std::pair<TIME_UNIT,std::vector<TIME_UNIT>>>  persched =  algorithms::generateKperiodicSchedule   (to , false) ;
	std::cerr << "Size = "<< persched.size() << std::endl;
	TIME_UNIT HP = 0.0;
	unsigned long LCM = 1;
	for (auto  key : persched) {
		auto task = key.first;
		//TIME_UNIT
		HP =    ( persched[task].first * to->getNi(task) ) / ( persched[task].second.size() *  to->getPhasesQuantity(task)) ;
		std::cout << "Task " <<  to->getVertexName(task) <<  " : duration=" <<  to->getVertexDuration(task) <<  " period=" <<  persched[task].first << " HP=" << HP << " Ni=" << to->getNi(task)<< " starts=[ ";

		LCM = boost::math::lcm(LCM, to->getNi(task));

		for (auto  skey : persched[task].second) {

			std::cout << skey << " " ;
		}
		std::cout << "]" << std::endl;

	}

	LCM = boost::math::lcm(LCM, (unsigned long)HP);
	std::cout << "LCM=" << LCM << "\n";

	std::cout <<  printers::PeriodicScheduling2DOT    (to, persched, false,  xscale , yscale);

	unsigned long max_router_size = 0;
	for (auto  key : conflictEdges) {
		auto edge_id = key.first;
		auto mysize = conflictEdges[edge_id].size();
		max_router_size = std::max(max_router_size, mysize);
		if(mysize > 1)
		{
			//std::cout << "potential conflict=" << mysize << "\n";
			for(unsigned int i = 0; i < mysize; i++)
			{
				for(unsigned int j = 0; j < mysize; j++)
				{
					if (i <= j) //ignore the upper triangle
						continue;
					auto taski = conflictEdges[edge_id][i].first;
					auto taskj = conflictEdges[edge_id][j].first;

					auto ni = to->getNi(taski);
					auto nj = to->getNi(taskj);

					auto srci = conflictEdges[edge_id][i].second;
					auto srcj = conflictEdges[edge_id][j].second;

					auto src_ni = to->getNi(srci);
					auto src_nj = to->getNi(srcj);

					//check if any of the starting times match
					std::vector<TIME_UNIT> si_vec, sj_vec;
					for (auto  skey : persched[taski].second) {
						si_vec.push_back(skey);
					}
					for (auto  skey : persched[taskj].second) {
						sj_vec.push_back(skey);
					}

					for(unsigned int si_i = 0; si_i < si_vec.size(); si_i++)
					{
						for(unsigned int sj_i = 0; sj_i < sj_vec.size(); sj_i++)
						{
							auto si =  persched[taski].second[si_i];
							auto sj =  persched[taskj].second[sj_i];

							if( isConflictPresent((LARGE_INT) HP, si, (LARGE_INT) ni, sj, (LARGE_INT) nj) )
							{
								std::cout << "conflict between " << to->getVertexName(taski) << " and " << to->getVertexName(taskj) << "\n";
								total_conflict += 1;

								addDependency(to, srci, srcj, src_ni, src_nj);

								// create a buffer 
								// the 
								break;
							}
						}
					}
				}
			}
		}
	}


	std::cout << "max_router_size=" << max_router_size << "\n";
	std::cout << "GGGGGGGGGGGG\n";


	std::cout << "total_conflict=" << total_conflict << "\n";
	to->reset_repetition_vector();
	VERBOSE_ASSERT(computeRepetitionVector(to),"inconsistent graph");

	std::map<Vertex,std::pair<TIME_UNIT,std::vector<TIME_UNIT>>>  persched2 =  algorithms::generateKperiodicSchedule   (to , false) ;
	for (auto  key : persched2) {
		auto task = key.first;
		//TIME_UNIT
		HP =    ( persched2[task].first * to->getNi(task) ) / ( persched2[task].second.size() *  to->getPhasesQuantity(task)) ;
		std::cout << "Task " <<  to->getVertexName(task) <<  " : duration=" <<  to->getVertexDuration(task) <<  " period=" <<  persched2[task].first << " HP=" << HP << " Ni=" << to->getNi(task)<< " starts=[ ";

		for (auto  skey : persched2[task].second) {

			std::cout << skey << " " ;
		}
		std::cout << "]" << std::endl;

	}

	for (auto  key : conflictEdges) {
			auto edge_id = key.first;
			auto mysize = conflictEdges[edge_id].size();
			if(mysize > 1)
			{
				//std::cout << "potential conflict=" << mysize << "\n";
				for(unsigned int i = 0; i < mysize; i++)
				{
					for(unsigned int j = 0; j < mysize; j++)
					{
						if (i <= j) //ignore the upper triangle
							continue;
						auto taski = conflictEdges[edge_id][i].first;
						auto taskj = conflictEdges[edge_id][j].first;

						auto ni = to->getNi(taski);
						auto nj = to->getNi(taskj);

						auto srci = conflictEdges[edge_id][i].second;
						auto srcj = conflictEdges[edge_id][j].second;

						auto src_ni = to->getNi(srci);
						auto src_nj = to->getNi(srcj);

						//check if any of the starting times match
						std::vector<TIME_UNIT> si_vec, sj_vec;
						for (auto  skey : persched[taski].second) {
							si_vec.push_back(skey);
						}
						for (auto  skey : persched[taskj].second) {
							sj_vec.push_back(skey);
						}

						for(unsigned int si_i = 0; si_i < si_vec.size(); si_i++)
						{
							for(unsigned int sj_i = 0; sj_i < sj_vec.size(); sj_i++)
							{
								auto si =  persched[taski].second[si_i];
								auto sj =  persched[taskj].second[sj_i];

								if( isConflictPresent((LARGE_INT) HP, si, (LARGE_INT) ni, sj, (LARGE_INT) nj) )
								{
									std::cout << "conflict between " << to->getVertexName(taski) << " and " << to->getVertexName(taskj) << "\n";
									total_conflict += 1;

									addDependency(to, srci, srcj, src_ni, src_nj);

									// create a buffer
									// the
									break;
								}
							}
						}
					}
				}
			}
		}
}


bool algorithms::isConflictPresent(LARGE_INT HP, TIME_UNIT si, LARGE_INT ni, TIME_UNIT sj, LARGE_INT nj)
{
	LARGE_INT temp1, temp2;
	LARGE_INT lhs = (LARGE_INT)(si - sj) * ni * nj;
	LARGE_INT rhs_gcd = HP * gcdExtended(myabs(ni), myabs(nj), &temp1, &temp2);

	if(si == sj) //if start time is same conflict
	{
		return true;
	}

	if (myabs(lhs) % rhs_gcd != 0)
	{
		return false; //No integral solutions exist as the division will yield floating point
	}
	else //check if any other solution exists
	{
		return true; //there exists a solution for sure
	}
}

