#!/perl/bin/perl

use strict;
use warnings;
use File::Basename;
use Cwd;
use Cwd 'abs_path';

my $exe = Cwd::realpath(shift);
my $path = Cwd::realpath(shift);
my $target = shift;

system("type nul>$target");

$target = Cwd::realpath($target);


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
    my $type = basename(dirname(abs_path($file)));
    if ($file=~ /.*$/) {
#	my $fichier = $name;
#	$fichier =~ s/(.+)\.[^.]+/$1/;
	system("echo|set /p dummy=\"$type;$name;\">> $target");
	$file =~s/\//\\\\/g;
	system("$exe $file>> $target");
    }
}

system("echo type;nom_instance;N;M;F;tps;solv;feas;opt;obj;obj1;obj2;obj22;setup;gapCplex;valide >> $target");
explore $path;
