#include "cpo2APC.h"

#include <algorithm>

void CpoSolver2APC::solve(ConfigAPC &config)
{
    AbstractSolverAPC::setUp(config);
    IloEnv env;
    try
    {
        IloOplErrorHandler handler(env, std::cout);
        IloOplModelSource modelSource(env, config.getModelPath().c_str());
        //IloOplModelSource modelSource(env, pathToOPL);
        IloOplSettings settings(env, handler);
        IloOplModelDefinition def(modelSource, settings);
        IloCP cp(env);
        IloOplModel opl(def, cp);
        MyCustomDataSource ds(env, problem);
        IloOplDataSource dataSource(&ds);
        opl.addDataSource(dataSource);
        opl.generate();
        if (config.isSilent())
        {
            cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
        }
        cp.setParameter(IloCP::TimeLimit, time_limit);

        //   Solution solSCH(P);
        //   Solution solQCH(P);
        //   if (withCPStart)
        // useCPStart(P,solSCH,solQCH,env,opl,cp);
        IloBool solCPFound = cp.solve();
        if (solCPFound)
        {
            IloOplElement elmt = opl.getElement("mjobs");
            modelToSol(env, cp, elmt);
            setSAT();
        }  
    }
    catch (IloOplException &e)
    {
        std::cout << "### OPL exception: " << e.getMessage() << std::endl;
        setERROR();
    }
    catch (IloException &e)
    {
        std::cout << "### exception: ";
        e.print(std::cout);
        setERROR();
    }
    catch (...)
    {
        std::cout << "### UNEXPECTED ERROR ..." << std::endl;
        setERROR();
    }
    env.end();
    AbstractSolverAPC::tearDown(config);
}

void MyCustomDataSource::read() const
{
    IloOplDataHandler handler = getDataHandler();
    const int F = problem.getFamilyNumber();
    // initialize the int 'simpleInt'
    handler.startElement("nbM");
    handler.addIntItem(problem.M);
    handler.endElement();
    handler.startElement("nbF");
    handler.addIntItem(F);
    handler.endElement();

    // initialize the int array 'simpleIntArray'
    handler.startElement("fsizes");
    handler.startArray();
    for (int f = 0; f < F; ++f)
    {
        handler.addIntItem(problem.getNf(f));
    }
    handler.endArray();
    handler.endElement();
    handler.startElement("durations");
    handler.startArray();
    for (int f = 0; f < F; ++f)
    {
        handler.addIntItem(problem.F[f].duration);
    }
    handler.endArray();
    handler.endElement();
    handler.startElement("thresholds");
    handler.startArray();
    for (int f = 0; f < F; ++f)
    {
        handler.addIntItem(problem.F[f].threshold);
    }
    handler.endArray();
    handler.endElement();

    //initialize a 2-dimension int array 'a2DIntArray'
    handler.startElement("setups");
    handler.startArray();
    for (int f = 0; f < F; f++)
    {
        handler.startArray();
        for (int j = 0; j < F; j++)
            if (f == j)
                handler.addIntItem(0);
            else
                handler.addIntItem(problem.F[j].setup);
        handler.endArray();
    }
    handler.endArray();
    handler.endElement();

    handler.startElement("qualifications");
    handler.startArray();
    for (int f = 0; f < F; f++)
    {
        handler.startArray();
        for (int j = 0; j < problem.M; j++)
            if (problem.F[f].qualif[j])
                handler.addIntItem(1);
            else
                handler.addIntItem(0);
        handler.endArray();
    }
    handler.endArray();
    handler.endElement();
}

int CpoSolver2APC::solToModel(Solution& solution, IloEnv &env, IloOplModel &opl, IloCP &cp)
{
    IloSolution sol(env);
    solution.reaffectId(problem);
    std::sort(solution.S.begin(), solution.S.end(), idComp);

    sol.setValue((opl.getElement("flowtime")).asIntVar(), solution.getSumCompletion(problem));
    sol.setEnd((opl.getElement("cmax")).asIntervalVar(), solution.getMaxEnd(problem));

    sol.setValue((opl.getElement("qualified")).asIntVar(), solution.getNbQualif(problem));

    IloIntervalVarMap jobs = opl.getElement("jobs").asIntervalVarMap();
    for (IloInt j = 1; j <= problem.N; ++j)
    {
        IloIntervalVar job_j = jobs.get(j);
        sol.setStart(job_j, solution.S[j - 1].start);
    }

    IloIntervalVarMap mjobs = opl.getElement("mjobs").asIntervalVarMap();
    for (IloInt j = 0; j < problem.N; ++j)
    {
        IloIntervalVarMap sub = mjobs.getSub(j + 1);
        for (IloInt k = 0; k < problem.M; ++k)
        {
            IloIntervalVar alt_job = sub.get(k + 1);
            if (solution.S[j].machine == k)
            {
                sol.setPresent(alt_job);
                //	sol.setStart(alt_job, solution.S[j].start);
            }
            else
                sol.setAbsent(alt_job);
        }
    }

    cp.setStartingPoint(sol);

    return 0;
}

int CpoSolver2APC::modelToSol(const IloEnv &env, const IloCP &cp, const IloOplElement &elmt)
{
    IloInt i, j;
    IloIntervalVarMap mjobs = elmt.asIntervalVarMap();
    IloIntervalVarMatrix dk(env, problem.N);
    for (i = 0; i < problem.N; ++i)
    {
        dk[i] = mjobs[i + 1].asNewIntervalVarArray();
    }

    for (i = 0; i < problem.N; ++i)
        for (j = 0; j < problem.M; ++j)
            if (problem.isQualif(i, j))
            {
                if (cp.isPresent(dk[i][j]))
                {
                    solution.S[i].index = i;
                    solution.S[i].start = (int)cp.getStart(dk[i][j]);
                    solution.S[i].machine = j;
                }
            }
    solution.repairDisqualif(problem);
    return 0;
}