#include "schedAPC.h"

int solve(const Problem& P, Solution& /*s*/){
  IloEnv env;

    int status = 127;
    try {
      IloOplErrorHandler handler(env,std::cout);
        IloOplModelSource modelSource(env,"schedAPC.mod");
        IloOplSettings settings(env,handler);
        IloOplModelDefinition def(modelSource,settings);
    	IloCP cp(env);
        IloOplModel opl(def,cp);
	MyCustomDataSource ds(env,P);
	IloOplDataSource dataSource(&ds);
	opl.addDataSource(dataSource);
	opl.generate();

           if ( cp.solve()) {
            std::cout << std::endl 
                << "OBJECTIVE: "  << opl.getCP().getObjValue() 
                << std::endl;
            opl.postProcess();
            opl.printSolution(std::cout);
            status = 0;
        } else {
            std::cout << "No solution!" << std::endl;
            status = 1;
        }
    } catch (IloOplException & e) {
        std::cout << "### OPL exception: " << e.getMessage() << std::endl;
    } catch( IloException & e ) {
        std::cout << "### exception: ";
        e.print(std::cout);
        status = 2;
    } catch (...) {
        std::cout << "### UNEXPECTED ERROR ..." << std::endl;
        status = 3;
    }

    env.end();
    
    return status;
}


void MyCustomDataSource::read() const {
  IloOplDataHandler handler = getDataHandler();
  const int F = P.getFamilyNumber();
  // initialize the int 'simpleInt'
  handler.startElement("nbM");
  handler.addIntItem(P.M);
  handler.endElement();
  handler.startElement("nbF");
  handler.addIntItem(F);
  handler.endElement();

  // initialize the int array 'simpleIntArray'
  handler.startElement("fsizes");
  handler.startArray();
  for (int f = 0 ; f < F ; ++f){
    handler.addIntItem(P.getNf(f));
  }
  handler.endArray();
  handler.endElement();
  handler.startElement("durations");
  handler.startArray();
  for (int f = 0 ; f < F ; ++f){
    handler.addIntItem(P.F[f].duration);
  }
  handler.endArray();
  handler.endElement();
  handler.startElement("thresholds");
  handler.startArray();
  for (int f = 0 ; f < F ; ++f){
    handler.addIntItem(P.F[f].threshold);
  }
  handler.endArray();
  handler.endElement();

  //initialize a 2-dimension int array 'a2DIntArray'
  handler.startElement("setups");
  handler.startArray();
  for (int f = 0 ; f < F ; f++) {
    handler.startArray();
    for (int j = 0 ; j < F ; j++)
      if (f == j ) handler.addIntItem(0);
      else handler.addIntItem(P.F[f].setup);
    handler.endArray();
  }  
  handler.endArray();
  handler.endElement();

  handler.startElement("qualifications");
  handler.startArray();
  for (int f = 0 ; f < F ; f++) {
    handler.startArray();
    for (int j = 0 ; j < P.M ; j++)
      if (P.F[f].qualif[j]) handler.addIntItem(1);
      else handler.addIntItem(0);
    handler.endArray();
  }  
  handler.endArray();
  handler.endElement();

}


/*int modelToSol(const Problem &, Solution&, const IloCP&, const IloIntervalVarMatrix&,const IloIntervalVarMatrix&){return 0;}

int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, const IloNum& bestObj,const IloNum& timeBestSol){return 0;}
*/
