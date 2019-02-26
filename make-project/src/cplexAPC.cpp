#include "cplexAPC.h"


void CplexSolverAPC::doSolve(IloEnv &env, ConfigAPC &config)
{
    const int F = problem.getFamilyNumber();
    const int T = problem.computeHorizon();
    IloModel model(env);

    IloNumVar3DMatrix x(env, F);
    IloNumVar3DMatrix y(env, F);
    IloNumVarArray C(env, F, 0, problem.N * (T + 1), ILOFLOAT);
    IloNumVarMatrix Y(env, F);

    createModel(T, env, model, x, y, C, Y);
    IloCplex cplex(model);
    configure(env, cplex, config);
    for (Solution sol : solutionPool)
    {
        IloNumVarArray startVar(env);
        IloNumArray startVal(env);
        solToModel(sol, x, y, Y, C, startVar, startVal);
        cplex.addMIPStart(startVar, startVal);
        startVar.end();
        startVal.end();
    }
    //timer.stageTimer();
    IloBool solMIPFound = cplex.solve();
    //timer.stopTimer();
    if (solMIPFound)
    {
        modelToSol(cplex, x, y, Y);
        solutionCount += cplex.getSolnPoolNsolns();
    }
    setStatus(cplex);
    tearDown(cplex);
}

void CplexSolverAPC::modelToSol(const IloCplex &cplex, const IloNumVar3DMatrix &x, const IloNumVar3DMatrix &y, const IloNumVarMatrix &Y)
{

    const int F = problem.getFamilyNumber();
    const int T = problem.computeHorizon();
    std::vector<int> found(F, 0);

    for (int i = 0; i < problem.N; ++i)
    {
        int cpt = 0;
        int j = 0;
        int trouve = 0;
        while (j < problem.M && !trouve)
        {
            if (problem.isQualif(i, j))
            {
                int t = 0;
                while (t < T - problem.getDuration(i) && !trouve)
                {
                    if (IloRound(cplex.getValue(x[problem.famOf[i]][j][t])) == 1)
                    {
                        ++cpt;
                        if (cpt == found[problem.famOf[i]] + 1)
                        {
                            solution.S[i] = Assignment(t, j, i);
                            found[problem.famOf[i]]++;
                            trouve = 1;
                        }
                    }
                    ++t;
                }
            }
            ++j;
        }
    }

    for (int f = 0; f < F; ++f)
        for (int j = 0; j < problem.M; ++j)
            if (problem.F[f].qualif[j] && IloRound(cplex.getValue(Y[f][j])))
                for (int t = 0; t < T; ++t)
                    if (IloRound(cplex.getValue(y[f][j][t])) == 1)
                    {
                        solution.QualifLostTime[f][j] = t;
                        break;
                    }
    solution.repairDisqualif(problem);
}

void CplexSolverAPC::solToModel(Solution &solution, IloNumVar3DMatrix &x, IloNumVar3DMatrix &y, IloNumVarMatrix &Y, IloNumVarArray &C, IloNumVarArray &startVar, IloNumArray &startVal)
{
    const int T = problem.computeHorizon();
    const int F = problem.getFamilyNumber();
    const int M = problem.M;

    //remplissage de x
    for (int i = 0; i < problem.N; ++i)
    {
        startVar.add(x[problem.famOf[solution.S[i].index]][solution.S[i].machine][solution.S[i].start]);
        startVal.add(1);
    }

    //remplissage de y
    for (int f = 0; f < F; ++f)
        for (int j = 0; j < M; ++j)
            if (problem.F[f].qualif[j])
            {
                if (solution.QualifLostTime[f][j] < std::numeric_limits<int>::max())
                {
                    int t = 0;
                    while (t < T)
                    {
                        startVar.add(y[f][j][t]);
                        if (t < solution.QualifLostTime[f][j])
                            startVal.add(0);
                        else
                            startVal.add(1);
                        ++t;
                    }
                }
            }

    //remplissage de Y
    for (int f = 0; f < F; ++f)
        for (int j = 0; j < M; ++j)
        {
            if (problem.F[f].qualif[j])
            {
                startVar.add(Y[f][j]);
                if (solution.QualifLostTime[f][j] == std::numeric_limits<int>::max())
                    startVal.add(0);
                else
                    startVal.add(1);
            }
        }

    //remplissage de C
    for (int f = 0; f < problem.getFamilyNumber(); ++f)
    {
        int sum = 0;
        for (int i = 0; i < problem.N; ++i)
            (problem.famOf[solution.S[i].index] == f ? sum += solution.S[i].start + problem.F[f].duration : sum = sum);
        startVar.add(C[f]);
        startVal.add(sum);
    }
}

void CplexSolverAPC::createModel(int T, IloEnv &env, IloModel &model,
                                 IloNumVar3DMatrix &x, IloNumVar3DMatrix &y, IloNumVarArray &C,
                                 IloNumVarMatrix &Y)
{
    createVars(T, env, x, y, Y);
    createConstraints(T, env, model, x, y, C, Y);
}

void CplexSolverAPC::createVars(int T, IloEnv &env, IloNumVar3DMatrix &x, IloNumVar3DMatrix &y, IloNumVarMatrix &Y)
{
    int i, j;

    IloNumArray Y_ub(env, problem.M);

    char namevar[24];
    for (i = 0; i < problem.getFamilyNumber(); ++i)
    {
        x[i] = IloNumVarMatrix(env, problem.M);
        for (j = 0; j < problem.M; ++j)
        {
            x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
            for (int t = 0; t < T; ++t)
            {
                snprintf(namevar, 24, "x_%d_%d_%d", i, t, j);
                x[i][j][t].setName(namevar);
            }
        }
    }
    for (i = 0; i < problem.getFamilyNumber(); ++i)
    {
        y[i] = IloNumVarMatrix(env, problem.M);
        for (j = 0; j < problem.M; ++j)
        {
            if (problem.F[i].qualif[j])
            {
                y[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
                Y_ub[j] = 1;
            }
            else
            {
                y[i][j] = IloNumVarArray(env, T, 0, 0, ILOINT);
                Y_ub[j] = 0;
            }
            for (int t = 0; t < T; ++t)
            {
                snprintf(namevar, 24, "y_%d_%d_%d", i, t, j);
                y[i][j][t].setName(namevar);
            }
        }

        Y[i] = IloNumVarArray(env, 0, Y_ub, ILOINT);
        for (int j = 0; j < problem.M; ++j)
        {
            snprintf(namevar, 24, "Y_%d_%d", i, j);
            Y[i][j].setName(namevar);
        }
    }
}

void CplexSolverAPC::createConstraints(int T, IloEnv &env, IloModel &model,
                                       IloNumVar3DMatrix &x, IloNumVar3DMatrix &y, IloNumVarArray &C,
                                       IloNumVarMatrix &Y)
{
    int j, t, f, tau;
    const int m = problem.M;
    const int F = problem.getFamilyNumber();
    std::vector<int> nf(F);
    for (int q = 0; q < F; ++q)
        nf[q] = problem.getNf(q);

    //objective
    // TODO Use config !
    IloExpr expr(env);
    for (f = 0; f < F; ++f)
        expr += alpha_C * C[f];
    for (f = 0; f < F; ++f)
        for (j = 0; j < m; ++j)
            if (problem.F[f].qualif[j])
                expr += beta_Y * Y[f][j];
    model.add(IloMinimize(env, expr));
    expr.end();

    //each job is scheduled once
    for (f = 0; f < F; ++f)
    {
        IloExpr expr(env);
        for (j = 0; j < m; ++j)
        {
            if (problem.F[f].qualif[j])
            {
                for (t = 0; t <= T - problem.F[f].duration; ++t)
                    expr += x[f][j][t];
            }
        }
        model.add(expr == nf[f]);
        expr.end();
    }

    //completion time of a job
    for (f = 0; f < F; ++f)
    {
        IloExpr expr(env);
        for (j = 0; j < m; ++j)
        {
            if (problem.F[f].qualif[j])
            {
                for (t = 0; t <= T - problem.F[f].duration; ++t)
                    expr += ((t + problem.F[f].duration) * x[f][j][t]);
            }
        }
        model.add(expr <= C[f]);
        expr.end();
    }

    //noOverlap on the same machine for to job of the same family
    for (f = 0; f < F; ++f)
        for (t = problem.F[f].duration - 1; t <= T - problem.F[f].duration; ++t)
            for (j = 0; j < m; ++j)
                if (problem.F[f].qualif[j])
                {
                    IloExpr expr(env);
                    for (tau = t - problem.F[f].duration + 1; tau <= t; ++tau)
                        expr += x[f][j][tau];
                    expr += y[f][j][t];
                    model.add(expr <= 1);
                    expr.end();
                }

    //noOverlap on the same machine with setup time
    for (f = 0; f < F; ++f)
        for (int f2 = 0; f2 < F; ++f2)
        {
            if (f != f2)
            {
                for (t = 0 /*problem.F[f].duration + problem.F[f2].setup - 1*/; t <= T - problem.F[f2].duration; ++t)
                    for (j = 0; j < m; ++j)
                        if (!((problem.F[f].qualif[j] + problem.F[f2].qualif[j]) % 2))
                        {
                            IloExpr expr(env);
                            for (tau = std::max(0, t - problem.F[f].duration - problem.F[f2].setup + 1);
                                 tau <= t; ++tau)
                                expr += x[f][j][tau];
                            expr += nf[f] * x[f2][j][t];
                            model.add(expr <= nf[f]);
                            expr.end();
                        }
            }
        }

    //threshold
    for (f = 0; f < F; ++f)
        for (j = 0; j < m; ++j)
            if (problem.F[f].qualif[j])
            {
                for (t = problem.F[f].threshold; t < T; ++t)
                {
                    IloExpr expr(env);
                    for (tau = t - problem.F[f].threshold + 1; tau <= t; ++tau)
                        expr += x[f][j][tau];
                    expr += y[f][j][t];
                    model.add(expr >= 1);
                    expr.end();
                }
            }

    for (t = 1; t < T; ++t)
        for (f = 0; f < F; ++f)
            for (j = 0; j < m; ++j)
                if (problem.F[f].qualif[j])
                {
                    IloExpr expr(env);
                    for (int f2 = 0; f2 < F; ++f2)
                    {
                        for (tau = std::max(0, t - problem.F[f2].duration); tau < T; ++tau)
                        {
                            for (int j2 = 0; j2 < m; ++j2)
                            {
                                if (problem.F[f2].qualif[j2])
                                {
                                    expr += x[f2][j2][tau];
                                }
                            }
                        }
                    }
                    expr = expr / (T - t);
                    expr += y[f][j][t - 1] - 1;
                    model.add(expr <= Y[f][j]);
                    expr.end();
                }

    //if a machine become disqualified, it stays disqualified
    for (t = 1; t < T; ++t)
        for (f = 0; f < F; ++f)
            for (j = 0; j < m; ++j)
                if (problem.F[f].qualif[j])
                    model.add(y[f][j][t - 1] <= y[f][j][t]);
}
