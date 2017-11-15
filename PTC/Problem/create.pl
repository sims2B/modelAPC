#!/usr/bin/perl

use strict;
use warnings;
use Cwd;

my $target = Cwd::realpath(shift);

sub handle{
    my $j = 2;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (10){
	    foreach my $f (2,3){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    }
    $j = 2;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (20,30){
	    foreach my $f (2,3,4,5){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    }
    $j = 3;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (40,50){
	    foreach my $f (2,3,4,5){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    }  
    $j = 4;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (70,90){
	    foreach my $f (2,3,4,5){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    } 
    $j = 5;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (100,120){
	    foreach my $f (2,3,4,5){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    }  
    $j = 6;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (150){
	    foreach my $f (2,3,4,5){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    }  
    $j = 8;
    for (my $k = 0; $k < 10 ; $k++){
	foreach my $i (200){
	    foreach my $f (2,3,4,5){
		system("./bin/le_nom $target/instance\_$i\_$j\_$f\_Mthr\_\_$k.txt $i $j $f");
	    }
	}
    }
    
}

handle;
