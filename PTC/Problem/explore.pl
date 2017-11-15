#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;
use Cwd;

my $path = Cwd::realpath(shift);
my $target = Cwd::realpath(shift);
my $target2 = Cwd::realpath(shift);

sub explore {
    my $p=shift;
    next if ($p =~ /.*\/\.[^\/]*$/);
    $p= Cwd::realpath $p;
    if (-d $p) {
	my $dir;
	opendir $dir,$p or die "Cannot open directory: $p";
	while (my $file=readdir($dir)) { explore("$p/$file"); }
    }
    elsif (-f $p) { handle_file($p); }
    else {warn "Neither file nor directory??? $p\n"; }
}

sub handle_file {
    my $file = shift;
    my $name = basename($file);
    if ($file=~ /.*$/) {
			my $fichier = $name;
			$fichier =~ s/(.+)\.[^.]+/$1/;
	    system("echo '$name' >> $target2");
	    system("timeout --signal=9 7200s ./bin/le_nom $file $target/$fichier\_TW\_ERint.txt >> $target2 2>&1");  
	    system("echo '**********************************************************'>> $target2");
    }
}

explore $path;
