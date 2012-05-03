use GraphViz;
use XML::Bare;

# Global GraphViz object
my $g;
my $uid = 0;
my %proc_node;
my @thread_colors = ('red', 'blue', 'green');
my $thread_cur = 0;

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

	my $proccluster = {
			name      =>"$pid @ $host",
			style     =>'filled',
			fillcolor =>'white',
	};

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


  # Get traces for first pass
  my @traces = @{$root->{process}->{traces}->{trace}};
	my @nodes;
	foreach $trace (@traces) {
		my $type = $trace->{type}->{value};
    if ($type eq "channel_connect") {
			my $call = $trace->{call}->{value}."$uid";
			my $tid = $trace->{tid}->{value};
      $uid++;
			push(@nodes, $call);
			$g->add_node($call,
									 label => $trace->{call}->{value},
									 cluster => $proccluster,
									 color => $thread_colors[$tid]);
    } elsif ($type eq "channel_write") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			my $call = $trace->{call}->{value}."$uid";
			my $tid = $trace->{tid}->{value};
			$proc_node{$host}{$pid}{$ipc}{$total_size} = $call;
      $uid++;
			push(@nodes, $call);
			$g->add_node($call,
 									 label => $trace->{call}->{value},
									 cluster => $proccluster,
									 color => $thread_colors[$tid]);
		} elsif ($type eq "channel_read") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			my $tid = $trace->{tid}->{value};
			my $call = $trace->{call}->{value}."$uid";
			$proc_node{$host}{$pid}{$ipc}{$total_size} = $call;
      $uid++;
			push(@nodes, $call);
			$g->add_node($call,
 									 label => $trace->{call}->{value},
									 cluster => $proccluster,
									 color => $thread_colors[$tid]);
		}
	}

  # Plot
	my $prev_node = "NULL";
  foreach $node (@nodes) {
		if ($prev_node ne "NULL") {
			$g->add_edge($prev_node, $node, arrowhead => 'empty');
		}
		$prev_node = $node;
	}
}

sub dump_proc_node() {
	print "===== Traces =====\n";
	foreach my $host (keys %proc_node) {
		print "==== $host ====\n";
    foreach my $pid (keys %{$proc_node{$host}}) {
			print "=== pid=$pid ===\n";
			foreach my $ipc (keys %{$proc_node{$host}{$pid}}) {
				print "== $ipc ==\n";
				foreach my $size (keys %{$proc_node{$host}{$pid}{$ipc}}) {
					print "uid: $size; ";
					print $proc_node{$host}{$pid}{$ipc}{$size}, "\n";
				}
			}
		}
	}
}

# Match inter-process read/write pairs.
# Each proc has such mapping: hostname -> pid -> mechanism->total_size -> dot node id
sub build_multiple_processes() {
  my $file = shift @_;
  my $obj = new XML::Bare(file => $file);
  my $root = $obj->parse();
  my $pid = $root->{process}->{header}->{pid}->{value};
	my $host = $root->{process}->{header}->{host}->{value};
	print "===== Match for $pid @ $host =====\n";
  my @traces = @{$root->{process}->{traces}->{trace}};
	foreach $trace (@traces) {
		my $type = $trace->{type}->{value};
    if ($type eq "channel_connect") {
		  # my $ipc = $trace->{mechanism}->{value};
      # TODO			
    } elsif ($type eq "channel_write") {
		  my $ipc = $trace->{mechanism}->{value};
			my $total_size = $trace->{total_size}->{value};
			my $target_host = $trace->{target}->{host}->{value};
			my $target_pid = $trace->{target}->{pid}->{value};
			my $target_call = $proc_node{$target_host}{$target_pid}{$ipc}{$total_size};
			my $call = $proc_node{$host}{$pid}{$ipc}{$total_size};
			$g->add_edge($call, $target_call, style => 'dotted');
		}
	}
}

#--------------------------------------------------
# Main
#--------------------------------------------------
my @file = ("sample_xml/host1.xml",
            "sample_xml/host2.xml");

$g = GraphViz->new(rankdir => 1,
                   node    => {fontcolor => 'white', style => 'filled'});


foreach $file (@file) {
  &build_single_process($file);
}
#&dump_proc_node();
foreach $file (@file) {
  &build_multiple_processes($file);
}

print $g->as_text();
open FILE, ">sample_xml/sample.ps";
print FILE $g->as_ps();
close FILE;
