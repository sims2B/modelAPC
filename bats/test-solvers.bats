
function assertSolvers() {
    gridjobs -l
    gridres -k table-keys.txt
    rm results/results.res
    ORACLE=`ls -1 results/*.res | head -n 1`
    for RES in `ls results/*.res`
    do
            diff -b -q -s $ORACLE $RES
    done
}


@test "Solver objective : Flow - Qual" {
    rm algorithms
    ln -s algorithmsFQ algorithms
    assertSolvers 
}

@test "Solver objective : Qual - Flow" {
    rm algorithms
    ln -s algorithmsQF algorithms
    ## assertSolvers 
}
