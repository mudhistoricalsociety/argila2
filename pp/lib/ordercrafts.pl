#!/usr/bin/perl -w
use strict;

my $filename = $ARGV[0];
my $endfile = "";
my %content;
my $footer;

open (CRAFTFILE, $filename);
$a = (<CRAFTFILE>);
while (($a) && ($a !~ /^craft/))
{
	if ($a !~ /^craft/)
	{
		$endfile .= $a; 
	}
	$a = (<CRAFTFILE>);
}

while (($a) && ($a =~ /craft\s+(\b\S+\b)\s+(\b\S+\b)\s+(\b\S+\b)/))
{
	my $handle = $1 . " " . $3;
	$content{$handle} = $a;
	$a = (<CRAFTFILE>);
	while (($a) && ($a !~ /^end/))
	{
		$content{$handle} .= $a;
		$a = (<CRAFTFILE>);
	}
	$content{$handle} .= $a;
	$a = (<CRAFTFILE>);
}

$footer .= $a;
while (<CRAFTFILE>)
{
	$footer .= $_;
}

my @hands = keys %content;
@hands = sort @hands;

my $elem;
foreach $elem (@hands)
{
	$endfile .= $content{$elem};
}

$endfile .= $footer;

close CRAFTFILE;
open (NEWCRAFTFILE, ">" . $filename);
print NEWCRAFTFILE $endfile;
