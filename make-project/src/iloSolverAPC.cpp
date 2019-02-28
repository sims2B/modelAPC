#include "iloSolverAPC.h"

void IloSolverAPC::setUp() {
    timer.stage();
    AbstractSolverAPC::setUp();
}
void IloSolverAPC::configure(IloEnv &env, IloCP &cp, ConfigAPC &config)
{
    if (config.isSilent())
    {
           std::cout << "c SILENT "  << std::endl;
        cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    }
    int timeLimit = config.getTimeLimit();
    if (timeLimit > 0)
    {
        cp.setParameter(IloCP::TimeLimit, timeLimit);
    }
    int workers = config.getWorkers();
    if (workers > 0)
    {
        cp.setParameter(IloCP::Workers, workers);
    }
}
void IloSolverAPC::tearDown(IloCP &cp)
{
    std::cout << "d STATUS " << cp.getStatus() << std::endl;
    std::cout << "d BRANCHES " << cp.getInfo(IloCP::NumberOfBranches) << std::endl;
    std::cout << "d FAILS " << cp.getInfo(IloCP::NumberOfFails) << std::endl;
    std::cout << "c VARIABLES " << cp.getInfo(IloCP::NumberOfVariables) << std::endl;
    std::cout << "c CONSTRAINTS " << cp.getInfo(IloCP::NumberOfConstraints) << std::endl;
    if (cp.getInfo(IloCP::NumberOfSolutions) > 0)
    {
        std::cout << "c GAP " << cp.getObjGap() << std::endl;
    }
}

void IloSolverAPC::tearDown() {
    AbstractSolverAPC::tearDown();
    if (hasSolution() && config.useTikzExport())
    {
        solution.toTikz(problem);
    }
}

void IloSolverAPC::configure(IloEnv &env, IloCplex &cplex, ConfigAPC &config)
{
    if (config.isSilent())
    {   
        cplex.setOut(env.getNullStream());
    }
    int timeLimit = config.getTimeLimit();
    if (timeLimit > 0)
    {
        cplex.setParam(IloCplex::TiLim, timeLimit);
    }
    int workers = config.getWorkers();
    if (workers > 0)
    {
        cplex.setParam(IloCplex::Threads, workers);
    }
}

void IloSolverAPC::setStatus(IloAlgorithm &cplex)
{
    auto cplexStatus = cplex.getStatus();
    if (solutionPool.size() > 0)
    {
        switch (cplexStatus)
        {
        case IloAlgorithm::Status::Optimal:
        case IloAlgorithm::Status::Infeasible:
        {
            status = S_OPTIMUM;
            break;
        }
        case IloAlgorithm::Status::Feasible:
        case IloAlgorithm::Status::Unknown:
        case IloAlgorithm::Status::InfeasibleOrUnbounded:
        {
            status = S_SAT;
            break;
        }
        default:
        {
            status = S_ERROR;
            break;
        }
        }
    }
    else
    {
        switch (cplexStatus)
        {
        case IloAlgorithm::Status::Optimal:
        {
            status = S_OPTIMUM;
            break;
        }
        case IloAlgorithm::Status::Infeasible:
        {
            status = S_UNSAT;
            break;
        }
        case IloAlgorithm::Status::Feasible:
        {
            status = S_SAT;
            break;
        }
        case IloAlgorithm::Status::Unknown:
        case IloAlgorithm::Status::InfeasibleOrUnbounded:
        {
            status = S_UNKNOWN;
            break;
        }
        default:
        {
            status = S_ERROR;
            break;
        }
        }
    }
}

// void IloSolverAPC::setStatus(bool hasSolution, bool hasReachedLimit)
// {
//     if (hasSolution)
//     {
//         status = hasReachedLimit ? S_SAT : S_OPTIMUM;
//     }
//     else
//     {
//         status = hasReachedLimit ? S_UNKNOWN : S_UNSAT;
//     }
// }

void IloSolverAPC::tearDown(IloCplex &cplex)
{
    std::cout << "d STATUS " << cplex.getStatus() << std::endl;
    std::cout << "c VARIABLES " << cplex.getNcols() << std::endl;
    std::cout << "c CONSTRAINTS " << cplex.getNrows() << std::endl;
    if (cplex.getSolnPoolNsolns() > 0)
    {
        std::cout << "d GAP " << cplex.getMIPRelativeGap() << std::endl;
    }
    std::cout << "d NBNODES " << cplex.getNnodes() << std::endl;
}

bool IloSolverAPC::iloSolve(IloAlgorithm &algo) {
    timer.stage("BUILD_TIME");
    IloBool solFound = algo.solve();
    timer.stage("RUNTIME");
    return solFound;
}

void IloSolverAPC::solve()
{
    timer.start();
    setUp();
    IloEnv env;
    try
    {
        solutionCount += solutionPool.size();
        // TODO timer.stage("BUILD_TIME");
        doSolve(env);
        // timer.stage("RUNTIME");
    }
    catch (IloOplException &e)
    {
        std::cout << "### OPL exception: " << e.getMessage() << std::endl;
        status = S_ERROR;
    }
    catch (IloException &e)
    {
        std::cout << "Iloexception in solve" << e << std::endl;
        e.end();
        status = S_ERROR;
    }
    catch (...)
    {
        std::cout << "Error unknown\n";
        status = S_ERROR;
    }
    env.end();

    if (hasSolution())
    {
        if (getSolutionCount() == (int)solutionPool.size())
        {
            // TODO Must set the best solution in the pool
            // DONE Restore the first solution
            std::cout << ">>>> TODO RESTORE BEST HEURISTIC SOLUTION <<<<" << std::endl;
            solution = solutionPool[0];
        }
    }
    tearDown();
    
}