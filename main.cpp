// Code to read graph instances from a file.  Uses the Boost Graph Library (BGL).
//This is the graph matching main file

#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include "d_random.h"

#define LargeValue 99999999

using namespace std;
using namespace boost;

int const NONE = -1;  // Used to represent a node that does not exist

struct VertexProperties;
struct EdgeProperties;

typedef adjacency_list<vecS, vecS, undirectedS, VertexProperties, EdgeProperties> Graph;

struct VertexProperties
{
    pair<int, int> cell; // maze cell (x,y) value
    Graph::vertex_descriptor pred;
    bool visited;
    bool marked;
    int weight;
};

// Create a struct to hold properties for each edge
struct EdgeProperties
{
    int weight;
    bool visited;
    bool marked;
};

int initializeGraph(Graph &g, ifstream &fin)
// Initialize g using data from fin.
{
   int numcolors;
   int n, e;
   int j, k;
   fin >> numcolors;
   fin >> n >> e;
   Graph::vertex_descriptor v;

   // Add nodes.
   for (int i = 0; i < n; i++)
      v = add_vertex(g);

   for (int i = 0; i < e; i++)
   {
      fin >> j >> k;
      add_edge(j, k, g);  // Assumes vertex list is type vecS
   }

   return numcolors;
}

void setNodeWeights(Graph &g, int w)
// Set all node weights to w.
{
   pair<Graph::vertex_iterator, Graph::vertex_iterator> vItrRange = vertices(g);

   for (Graph::vertex_iterator vItr = vItrRange.first; vItr != vItrRange.second; ++vItr)
   {
      g[*vItr].weight = w;
   }
}

vector<bool> validColors(Graph &g, int &nodeindex, int &numcolors, vector <int>
                                                                  &colorlist)
{
   vector<bool> validcolors(numcolors, true);
   Graph::vertex_iterator vertexIt, vertexEnd;
   Graph::in_edge_iterator inedgeIt, inedgeEnd;
   Graph::adjacency_iterator neighbourIt, neighbourEnd;

   tie(neighbourIt, neighbourEnd)=adjacent_vertices(nodeindex,g);
   for (; neighbourIt != neighbourEnd; ++neighbourIt)
   {
      validcolors[colorlist[*neighbourIt]] = false;
   }
   return validcolors;
}

void optimizeColorConflicts(Graph &g, int timelim, int& numcolors, string name)
{

   //initializes all necessary variables, including graph iterator
   int conflicts = 0, deg, maxdeg = 0, unselected = 0, random;
   vector<int> colors (num_vertices(g), 0);
   vector<bool> validcolors (num_vertices(g), true), selected (num_vertices(g), false);
   float elapsedtime = 0;
   time_t starttime = clock(), currtime = clock();
   vector <pair<int, int>> outdeg; //first int: outdegree, second int: node
   pair <int, int> vectordeg, currnode;
   randomNumber r;
   Graph::vertex_iterator vertexIt, vertexEnd;
   Graph::in_edge_iterator inedgeIt, inedgeEnd;
   Graph::adjacency_iterator neighbourIt, neighbourEnd;

   tie(vertexIt, vertexEnd) = vertices(g);
   for (; vertexIt != vertexEnd; ++vertexIt)
   {
      deg = 0;
      vectordeg.second = (int)*vertexIt;
      tie(inedgeIt, inedgeEnd) = in_edges(*vertexIt, g);

      for(; inedgeIt != inedgeEnd; ++inedgeIt)
         deg++;

      vectordeg.first = deg;
      outdeg.push_back(vectordeg);

      if (deg > maxdeg)
         maxdeg = deg;
      unselected++;
   }

   sort(outdeg.begin(), outdeg.end());

   while(unselected > 0) {

      currnode = outdeg.back();
      outdeg.pop_back();

      validcolors = validColors(g, currnode.second, numcolors, colors);

      if (std::none_of(validcolors.begin(), validcolors.end(),
                       [](bool v) { return v; }))
         colors[currnode.second] = (int) abs(r.random()) % numcolors;
      else {
         do {
            random = (int) abs(r.random()) % numcolors;

            if (random < 0)
               random = abs(random) % numcolors;

         } while (validcolors[random] == false);
         colors[currnode.second] = random;
      }

      unselected--;
   }

   tie(vertexIt, vertexEnd) = vertices(g);
   for (; vertexIt != vertexEnd; ++vertexIt) {

      tie(neighbourIt, neighbourEnd) = adjacent_vertices(*vertexIt, g);
      for (; neighbourIt != neighbourEnd; ++neighbourIt) {
         if (colors[*neighbourIt] == colors[*vertexIt]) {
            conflicts++;
         }
      }
   }

   currtime = clock();
   elapsedtime = (float)(currtime - starttime)/CLOCKS_PER_SEC;
   ofstream myfile;
   string out = ".output";
   myfile.open((name + out));
   cout << "fewest number of conflicts: " << conflicts << endl;
   myfile << "fewest number of conflicts: " << conflicts << endl;
   for (int bestsol = 0; bestsol < num_vertices(g); bestsol++)
   {
      myfile << "node " << bestsol;
      myfile << " color: " << colors[bestsol] << endl;
   }
   return;

}

int main()
{
   char x, delim('.');
   ifstream fin;
   string ogfilename, filename, input;
   int stopper, c;
   // Read the name of the graph from the keyboard or
   // hard code it here for testing.


   //fileName = "color12-3.input";
   cout << "Enter filename" << endl;
   getline(cin, filename, delim);
   getline(cin, input);
   ogfilename = filename + '.' + input;
   fin.open(ogfilename.c_str());
   if (!fin) {
      cerr << "Cannot open " << ogfilename << endl;
      exit(1);
   }

   try {
      cout << "Reading graph" << endl;
      Graph g;
      int colorlimit = initializeGraph(g, fin);
      cout << "Num nodes: " << num_vertices(g) << endl;
      cout << "Num edges: " << num_edges(g) << endl;
      cout << endl;

      int timelim = 600;
      optimizeColorConflicts(g, timelim, colorlimit, filename);

      cout << "Graph has now been optimized.";
      return(0);
   }
   catch (int y) {
      cout << "An exception occured" << endl;
   }

}