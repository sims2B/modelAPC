#include "iloSolverAPC.h"

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

void IloSolverAPC::tearDown(IloCP &cp)
{

    std::cout << "c TODO ILO_CP_NOT_AVAIL"
              << "\n";
}

void IloSolverAPC::setStatus(IloCplex &cplex)
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

void IloSolverAPC::setStatus(bool hasSolution, bool hasReachedLimit)
{
    if (hasSolution)
    {
        status = hasReachedLimit ? S_SAT : S_OPTIMUM;
    }
    else
    {
        status = hasReachedLimit ? S_UNKNOWN : S_UNSAT;
    }
}

void IloSolverAPC::solve(ConfigAPC &config)
{
    IloEnv env;
    try
    {
        setUp(config);
        solutionCount += solutionPool.size();
        doSolve(env, config);
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
        solution.toTikz(problem);
    }
    AbstractSolverAPC::tearDown(config);
}