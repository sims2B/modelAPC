#!/usr/bin/perl

use strict;
use warnings;
use Cwd;


sub handle{
    my $cpt = 1;
    my $j = 2;
    foreach my $i (10){
	foreach my $f (2,3){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    }
    $j = 2;
    foreach my $i (20,30){
	foreach my $f (2,3,4,5){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    }
    $j =3;
    foreach my $i (40,50){
	foreach my $f (2,3,4,5){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    } 
    $j =4;
    foreach my $i (70,90){
	foreach my $f (2,3,4,5){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    }   
    $j =5;
    foreach my $i (100,120){
	foreach my $f (2,3,4,5){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    }  
    $j =6;
    foreach my $i (150){
	foreach my $f (2,3,4,5){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    }    
    $j =8;
    foreach my $i (200){
	foreach my $f (2,3,4,5){
	    system("mkdir T$cpt\_$i\_$j\_$f\_big");
	    system("mv instance\_$i\_$j\_$f\_Bthr\_\_* T$cpt\_$i\_$j\_$f\_big");
	    $cpt = $cpt +1;
	}
    }
}

handle;
