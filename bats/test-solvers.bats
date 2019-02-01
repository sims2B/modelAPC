#EXECS='../PTC/CP/OPL/bin/schedAPC ../PTC/CP/jobBased/bin/modelCP1 ../PTC/MIP/IP1/bin/modelIP1 ../PTC/MIP/IP2/bin/modelIP2 ../PTC/MIP/IP3/bin/modelIP3'
EXECS='../PTC/CP/OPL/bin/schedAPC ../PTC/CP/jobBased/bin/modelCP1 ../PTC/MIP/IP3/bin/modelIP3'
    
function assertSolvers() {
    for EXE in $EXECS
    do
        run $EXE $1 
        [ "$status" -eq 0 ]
        flowtime=`echo $output | sed -e 's/^.*d FLOWTIME \([[:alnum:]]*\) .*/\1/'`
        # echo $flowtime >> test.log
        [ "$flowtime" = "$2" ]
        qualified=`echo $output | sed -e 's/^.*d QUALIFIED \([[:alnum:]]*\) .*/\1/'`
        # echo $qualified >> bats.log
        [ "$qualified" = "$3" ]        
    done

}


@test "Easy 10" {
    assertSolvers $BATS_TEST_DIRNAME/instances/instance_10_2_3_10_5_Sthr_0.txt 159 5
    assertSolvers $BATS_TEST_DIRNAME/instances/instance_10_2_3_10_5_Sthr_1.txt 74 5
    assertSolvers $BATS_TEST_DIRNAME/instances/instance_10_2_3_10_5_Sthr_2.txt 207 5
    assertSolvers $BATS_TEST_DIRNAME/instances/instance_10_2_3_10_5_Sthr_3.txt 162 5
    assertSolvers $BATS_TEST_DIRNAME/instances/instance_10_2_3_10_5_Sthr_4.txt 257 4
}
