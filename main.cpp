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
struct ColoringSet;

typedef adjacency_list<vecS, vecS, undirectedS, VertexProperties, EdgeProperties> Graph;
int numConflicts(Graph& g, vector<int> colors);

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

struct ColoringSet
{
    vector<int> colorset;
    int conflicts;
    ColoringSet(int numclr)
    {
       this->colorset = vector<int> (numclr);
    }

    ColoringSet()
    {
       this->conflicts = LargeValue;
    }

    bool operator < (const ColoringSet& clrst) const
    {
       return (conflicts < clrst.conflicts);
    }

    void operator = (const ColoringSet& clrst)
    {
       this->conflicts = clrst.conflicts;
       this->colorset = clrst.colorset;
    }

    void setCon(Graph &g, vector<int> clrst)
    {
       this->colorset = clrst;
       this->conflicts = numConflicts(g, this->colorset);
    }
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
// function takes in the graph, current node, current coloring, and number of
// possible colors.  It then computes which colors are still valid and if
// none are viable, retruns a completely false vector.
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

int numConflicts(Graph& g, vector<int> colors)
//returns the number of conflicts in a graph given the coloring scheme
{
   Graph::vertex_iterator vertexIt, vertexEnd;
   Graph::adjacency_iterator neighbourIt, neighbourEnd;
   int conflicts = 0;
   tie(vertexIt, vertexEnd) = vertices(g);
   for (; vertexIt != vertexEnd; ++vertexIt) {
      tie(neighbourIt, neighbourEnd) = adjacent_vertices(*vertexIt, g);
      for (; neighbourIt != neighbourEnd; ++neighbourIt) {
         if (colors[*neighbourIt] == colors[*vertexIt]) {
            conflicts++;
         }
      }
   }
   return conflicts;

}

vector<int> GreedyColorCon(Graph &g, int numcolors)
//creates a greedy coloring, to be used as a basis in local search
{

   //initializes all necessary variables, including graph iterator
   int conflicts = 0, deg, maxdeg = 0, unselected = 0, random;
   vector<int> colors (num_vertices(g), 0);
   vector<bool> validcolors (num_vertices(g), true), selected (num_vertices(g), false);
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
   // loops through all every node, starting with the most connected, and
   // colors each node
   while(unselected > 0) {
      //sets the current node to be the most conflicted yet assigned
      currnode = outdeg.back();
      outdeg.pop_back();

      // determines what colors are still valid options
      validcolors = validColors(g, currnode.second, numcolors, colors);

      //checks if every possible color option is conflicted
      if (std::none_of(validcolors.begin(), validcolors.end(),
                       [](bool v) { return v; }))
         colors[currnode.second] = (int) abs(r.random()) % numcolors;
         //otherwise, randomly picks a valid color to color the node
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
   return colors;

}

void steepestDescent(Graph g, vector<int> &colors)
// uses 2-opt to check if there is a better version of coloring than one passed
// if there is, it attempts to optimize that solution by recursion
// otherwise, it considers the coloring to be locally optimal
{
   vector<int> bestcolor = colors, initcolor = colors;
   Graph::vertex_iterator vertexIt, vertexEnd, swapIt, swapEnd;
   int tempcolor, conflicts, leastconflicts = numConflicts(g, colors);
   tie(vertexIt, vertexEnd) = vertices(g);
   tie(swapIt, swapEnd) = vertices(g);
   for (; vertexIt != vertexEnd; ++vertexIt) {
      for(; swapIt != swapEnd; ++swapIt) {
         if (swapIt != vertexIt)
         {
            colors = initcolor;
            tempcolor = colors[*swapIt];
            colors[*swapIt] = colors[*vertexIt];
            colors[*vertexIt] = tempcolor;
            conflicts = numConflicts(g, colors);
            if (conflicts < leastconflicts)
            {
               bestcolor = colors;
               leastconflicts = conflicts;
            }
         }
      }
   }
   if (bestcolor != initcolor) {
      steepestDescent(g, bestcolor);
      colors = bestcolor;
   }
   else {
      cout << "Optimal solution found!" << endl;
      cout << numConflicts(g, bestcolor) << endl;
      colors = bestcolor;
   }
   return;
}

int Genetic(Graph g, vector<int> &colors, int numcolor)
// use genetic algorithm to try and find an optimal solution
{
   clock_t timestart = clock(), timenow; //Set the start of the clock for timeout
   deque<ColoringSet> parents, children;
   ColoringSet currcolor(numcolor), bestcolor(numcolor);
   ColoringSet nextcolor(numcolor), newcolor(numcolor);
   Graph::vertex_iterator vertexIt, vertexEnd, swapIt, swapEnd;
   int conflicts = numConflicts(g, colors), r, s, z, same = 0, bestcon;
   float timeelapsed = 0;
   randomNumber randnum;
   tie(vertexIt, vertexEnd) = vertices(g);
   tie(swapIt, swapEnd) = vertices(g);
   newcolor.colorset = colors;
   newcolor.conflicts = conflicts;
   bestcolor = newcolor;
   parents.push_front(newcolor);
   for (int i = 0; i <= pow(colors.size(), 2); i++)
   {
      newcolor.setCon(g, GreedyColorCon(g, numcolor));
      parents.push_back(newcolor);
   }
   while (timeelapsed <= 300) {
      bestcon = bestcolor.conflicts;
      do
      {
         s = (int)(randnum.random() % colors.size());
         z = z % 2;
         if (parents.size() == 1)
            z = 0;

         if (z == 0) {
            //mutate a single solution
            currcolor = parents.front();
            parents.pop_front();

            //generate two random numbers, a node # and color, and assign new
            // color to chosen node, assuming it's a different color
            do
               r = (int) abs(randnum.random() % numcolor);
            while (r == currcolor.colorset[s]);
            currcolor.colorset[s] = r;
            currcolor.conflicts = numConflicts(g, currcolor.colorset);
         }
         else {
            //create offspring of two solutions
            newcolor = parents.front();
            parents.pop_front();
            nextcolor = parents.front();
            parents.pop_front();
            //merge two color sets
            for (int j = 0; j < colors.size(); j++) {
               if (j < s)
                  currcolor.colorset[j] = newcolor.colorset[j];
               else
                  currcolor.colorset[j] = nextcolor.colorset[j];
            }
            currcolor.conflicts = numConflicts(g, currcolor.colorset);
         }
         //check if new result is less than best result so far.
         if (currcolor.conflicts < bestcolor.conflicts)
            bestcolor = currcolor;

         children.push_back(currcolor);

         z++;
      } while (parents.size() >= 1);
      parents.clear();
      parents.resize(children.size());
      sort(children.begin(),children.end());
      for (int j = 0; j < children.size(); j++) {
         parents[j] = children[j];
      }
      children.clear();
      if (bestcolor.conflicts == 0)
      {
         cout << "found a legal coloring!" << endl;
         return 0;
      }
      timenow = clock();
      timeelapsed = (float)(timenow - timestart)/CLOCKS_PER_SEC;

      if (parents.front().conflicts == bestcon)
         same++;
      else
         same = 0;

      if (same >= 4)
         break;
   }
   colors = bestcolor.colorset;
   return bestcolor.conflicts;
}

void printSolution(string name, vector<int> colors, int conflicts)
{
   ofstream myfile;
   string out = ".output";
   myfile.open((name + out));
   cout << "fewest number of conflicts: " << conflicts << endl;
   myfile << "fewest number of conflicts: " << conflicts << endl;
   for (int bestsol = 0; bestsol < (int)colors.size(); bestsol++) {
      myfile << "node " << bestsol;
      myfile << " color: " << colors[bestsol] << endl;
   }
}

int main()
{
   char x, delim('.');
   ifstream fin;
   string ogfilename, filename, input;
   int stopper, c, conflicts;
   // Read the name of the graph from the keyboard or
   // hard code it here for testing.
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
      vector<int> colors = GreedyColorCon(g, colorlimit);
      //steepestDescent(g, colors);
      cout << "Graph has now been optimized.";
      conflicts = Genetic(g, colors, colorlimit);
      printSolution(filename, colors, conflicts);
      return(0);
   }
   catch (int y) {
      cout << "An exception occured" << endl;
   }
   /*
   currtime = clock();
   elapsedtime = (float)(currtime - starttime)/CLOCKS_PER_SEC;
   return;
    */
}