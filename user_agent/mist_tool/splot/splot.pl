use GraphViz;
use XML::Bare;

# Global GraphViz object
my $g = GraphViz->new();

sub build_single_process() {
  my $file = shift @_;
  my $obj = new XML::Bare(file => $file);
  my $root = $obj->parse();

  # Get Header
  my $header = $root->{process}->{header};
  my $pid = $header->{pid}->{value};
	my $host = $header->{host}->{value};
  my $exe_name = $header->{exe_name}->{value};
  my $cmdline_argument = $header->{cmdline_argument}->{value};
  my $parent_pid = $header->{parent}->{pid}->{value};
  my $parent_exe_name = $header->{parent}->{exe_name}->{value};
  my $parent_cmdline_argument = $header->{parent}->{cmdline_argument}->{value};

print <<ENDDUMP;
===== Header =====
Pid : $pid
Host: $host
Exe Name: $exe_name
Cmdline Arguments: $cmdline_argument
Parent Pid : $parent_pid
Parent Exe Name: $parent_exe_name
Parent Cmdline Arguments: $parent_cmdline_argument
ENDDUMP


  # Get traces for first pass
  my @traces = @{$root->{process}->{traces}->{trace}};
	my %proc_node;
	foreach $trace (@traces) {
		my $type = $trace->{type}->{value};
    if ($type eq "channel_connect") {
		  my $ipc = $trace->{mechanism}->{value};
    } elsif ($type eq "channel_write") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			$proc_node{$host}{$pid}{$ipc}{$total_size} = "write";
		} elsif ($tyep eq "channel_read") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			$proc_node{$host}{$pid}{$ipc}{$total_size} = "read";
		}
	}

  # Build inter-process communication graph
	foreach $trace (@traces) {
		my $type = $trace->{type}->{value};
    if ($type eq "channel_write") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			my $trg_pid = $trace->{target}->{pid}->{value};
			my $trg_host = $trace->{target}->{host}->{value};
			print $proc_node{$trg_host}{$trg_pid}{$ipc}{$total_size}, "\n";
		} elsif ($tyep eq "channel_read") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			print $proc_node{$trg_host}{$trg_pid}{$ipc}{$total_size}, "\n";
		}
	}
}

# Match inter-process read/write pairs.
# Each proc has such mapping: hostname -> pid -> mechanism->total_size -> dot node id
sub build_multiple_processes() {
}

#$g->add_node('London');
#$g->add_node('Paris', label => 'City of\nlurve');
#$g->add_node('New York');
#$g->add_edge('London' => 'Paris');
#$g->add_edge('London' => 'New York', label => 'Far');
#$g->add_edge('Paris' => 'London');
#print $g->as_text;

#--------------------------------------------------
# Main
#--------------------------------------------------
my $file = "sample_xml/sample.xml";

# foreach $file (@files) {
&build_single_process($file);
# }
&build_multiple_processes();
