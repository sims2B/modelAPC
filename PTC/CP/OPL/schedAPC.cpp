#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <ilopl/iloopl.h>
#include <sstream>

using namespace std;

int main(int argc,char* argv[]) {
    IloEnv env;

    int status = 127;
    try {
        IloOplErrorHandler handler(env,cout);
        IloOplModelSource modelSource(env,"schedAPC.mod");
        IloOplSettings settings(env,handler);
        IloOplModelDefinition def(modelSource,settings);
    	IloCP cp(env);
        IloOplModel opl(def,cp);
        IloOplDataSource dataSource(env, "instance_10_2_3_10_5_Bthr_0-opl.dat");
        opl.addDataSource(dataSource);
        opl.generate();

           if ( cp.solve()) {
            cout << endl 
                << "OBJECTIVE: "  << opl.getCP().getObjValue() 
                << endl;
            opl.postProcess();
            opl.printSolution(cout);
            status = 0;
        } else {
            cout << "No solution!" << endl;
            status = 1;
        }
    } catch (IloOplException & e) {
        cout << "### OPL exception: " << e.getMessage() << endl;
    } catch( IloException & e ) {
        cout << "### exception: ";
        e.print(cout);
        status = 2;
    } catch (...) {
        cout << "### UNEXPECTED ERROR ..." << endl;
        status = 3;
    }

    env.end();

    cout << endl << "--Press <Enter> to exit--" << endl;
    getchar();
    
    return status;
}
