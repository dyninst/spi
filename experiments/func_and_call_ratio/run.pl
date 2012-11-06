my $file = $ARGV[0];
open FILE, $file or die $!;
while (<FILE>) {
		print $_;
}
close FILE;
