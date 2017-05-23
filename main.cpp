// Code to read graph instances from a file.  Uses the Boost Graph Library (BGL).
//This is the graph matching main file

#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>

#define LargeValue 99999999

using namespace std;
using namespace boost;

int const NONE = -1;  // Used to represent a node that does not exist

struct VertexProperties;
struct EdgeProperties;

typedef adjacency_list<vecS, vecS, bidirectionalS, VertexProperties, EdgeProperties> Graph;

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

void optimizeColorConflicts(Graph &g, int timelim, int& numcolors, string name)
{
   int bestcon = LargeValue, conflicts = 0;
   vector<int> colors, bestcolors;
   float trials = 0;
   time_t starttime = clock(), currtime = clock();
   float elapsedtime = 0;

   colors.resize(num_vertices(g), 0);
   trials = (float)pow(numcolors, num_vertices(g)); //ex. 3 digit comb lock =
   // 10^3
   if (trials < 0)
      trials = LargeValue;
   Graph::vertex_iterator vertexIt, vertexEnd;
   Graph::in_edge_iterator inedgeIt, inedgeEnd;
   Graph::in_edge_iterator outedgeIt, outedgeEnd;
   Graph::adjacency_iterator neighbourIt, neighbourEnd;
   tie(vertexIt, vertexEnd) = vertices(g);
   vector <int> outdeg;
   int deg, unselected, maxdeg = 0, maxdegnode;
   for (; vertexIt != vertexEnd; ++vertexIt)
   {
      deg = 0;
      tie(inedgeIt, inedgeEnd) = in_edges(*vertexIt, g);
      for(; inedgeIt != inedgeEnd; ++inedgeIt)
         deg++;
      outdeg.push_back(deg);
      if (deg > maxdeg)
      {
         maxdeg = deg;
         maxdegnode = (int)*vertexIt;
      }
      unselected++;
   }
 while(unselected > 0)
 {

 }


   /*
   for (int i = 0; i < trials; i++) //runs for number of combinations
   {
      conflicts = 0;
      //check for conflicts

      int j = (int)num_vertices(g) - 1;
      int basenum = i;

      while (basenum != 0 && j > -1) // creates a base number for the colors
      {
         colors[j] = basenum % numcolors;
         basenum = basenum / numcolors;
         j--;
      }

      Graph::vertex_iterator vertexIt, vertexEnd;
      Graph::adjacency_iterator neighbourIt, neighbourEnd;
      tie(vertexIt, vertexEnd) = vertices(g);
      for (; vertexIt != vertexEnd; ++vertexIt)
      {
         tie(neighbourIt, neighbourEnd) = adjacent_vertices(*vertexIt, g);

         for (; neighbourIt != neighbourEnd; ++neighbourIt)
         {
            if (colors[*neighbourIt] == colors[*vertexIt])
               conflicts++;
         }
      }

      if (conflicts < bestcon) {
         bestcon = conflicts;
         bestcolors = colors;
      }
      currtime = clock();
      elapsedtime = (float)(currtime - starttime)/CLOCKS_PER_SEC;


      if (elapsedtime >= timelim || i == trials - 1 || conflicts == 0)
      {
         ofstream myfile;
         string out = ".output";
         myfile.open((name + out));
         myfile << "fewest number of conflicts: " << bestcon << endl;
         for (int bestsol = 0; bestsol < num_vertices(g); bestsol++)
         {
            myfile << "node color: " << bestcolors[bestsol] << endl;
         }
         return;
      }

   }
*/
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