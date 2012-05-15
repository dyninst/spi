use GraphViz;
use XML::Bare;

# Global GraphViz object
my $g;

#my $xml_dir = "server_xml";
my $xml_dir = $ARGV[0];

# cur_pid->parent_pid
my %pid_ppid_map;
# cur_pid->exe_name
my %pid_exe_map;
# parent_pid->exe_name
my %ppid_exe_map;

# Draw nodes, each of which is a process
sub parse_xml {
		my $file = "$xml_dir/".(shift @_);
		my $obj = new XML::Bare(file => $file);
		if (!$obj) {
				return;
		}
		my $root = $obj->parse();

		my $header = $root->{process}->{head};
		my $pid = $header->{pid}->{value};
		my $host = $header->{host}->{value};
		my $exe_name = $header->{exe_name}->{value};
		my $cmdline_argument = $header->{cmdline_argument}->{value};
		my $parent_pid = $header->{parent}->{pid}->{value};
		my $parent_exe_name = $header->{parent}->{exe_name}->{value};
		my $parent_cmdline_argument = $header->{parent}->{cmdline_argument}->{value};

print <<ENDDUMP;
===== Header for $pid @ $host =====
Pid : $pid
Host: $host
Exe Name: $exe_name
Cmdline Arguments: $cmdline_argument
Parent Pid : $parent_pid
Parent Exe Name: $parent_exe_name
Parent Cmdline Arguments: $parent_cmdline_argument
ENDDUMP

    $pid_ppid_map{$pid} = $parent_pid;
		if ($exe_name) {
				$pid_exe_map{$pid} = $exe_name;
		}
		if ($parent_exe_name) {
				$pid_exe_map{$parent_pid} = $parent_exe_name;
		}
}

# Draw edges
sub draw_graph {
		foreach $pid (keys %pid_exe_map) {
				$g->add_node($pid,
										 label => "$pid_exe_map{$pid} (pid=$pid)",
						         color => "red");
		}

		foreach $pid (keys %pid_ppid_map) {
				$ppid = $pid_ppid_map{$pid};
				$g->add_edge($ppid, $pid, style => 'dotted');
		}
}

#------------------------------------------------------------
# Main
#------------------------------------------------------------
$g = GraphViz->new(rankdir => 0,
                   node    => {fontcolor => 'white', style => 'filled'});

opendir DIR, $xml_dir or die $!;
my @files = readdir(DIR);
foreach $f (@files) {
		if ($f=~ m/.+\.xml/) {
				&parse_xml($f);
		}
}
closedir DIR;

&draw_graph();

print $g->as_text();
open FILE, ">$xml_dir/sample.png";
print FILE $g->as_png();
close FILE;
