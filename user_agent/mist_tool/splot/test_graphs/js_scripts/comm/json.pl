use GraphViz;
use XML::Bare;

#-------------------------------
# configure variables
#-------------------------------
my $cat = "comm_hier";
my $graph_name = "w_label";


my %colors;
$colors{"condor"} = "#FEE6CE";
$colors{"wenbin"} = "#31A354";
$colors{"root"} = "#FC9272";

my $json;

my $xml_dir = "../../../condor_traces";
my %mytraces;

# Build structures
sub parse_xml {
		my $file = "".(shift @_);
		my $obj = new XML::Bare(file => $file);
		if (!$obj) {
				return;
		}
		my $root = $obj->parse();

		my $header = $root->{process}->{head};
		my $pid = $header->{pid}->{value};
		my $host = $header->{host}->{value};
		my $user = $header->{effective_user}->{name}->{value};
		my $exe_name = $header->{exe_name}->{value};
		my $cmdline_argument = $header->{cmdline_argument}->{value};
		my $parent_pid = $header->{parent}->{pid}->{value};
		my $parent_exe_name = $header->{parent}->{exe_name}->{value};
		my $parent_user = $header->{parent}->{real_user}->{name}->{value};
		my $parent_cmdline_argument = $header->{parent}->{cmdline_argument}->{value};

print <<ENDDUMP;
===== Header for $pid @ $host =====
Pid : $pid
Host: $host
Exe Name: $exe_name
Cmdline Arguments: $cmdline_argument
User: $user
Parent Pid : $parent_pid
Parent Exe Name: $parent_exe_name
Parent Cmdline Arguments: $parent_cmdline_argument
ENDDUMP

    my @traces = @{$root->{process}->{traces}->{trace}};
		my @listens;
		my @targets;
		foreach $t (@traces) {
				my $type = $t->{type}->{value};
				if ($type eq "connect") {
						my $host = $t->{target}->{host}->{value};
						my $port = $t->{target}->{port}->{value};
						push(@targets, "$host:$port");
						print "connect to $host:$port\n";
				} elsif ($type eq "listen") {
						my $port = $t->{port}->{value};
						push(@listens, $port);
						print "listen to port $port\n";
				}
		}

		my %pid_bucket;
		$pid_bucket{"exe_name"} = $exe_name;
		$pid_bucket{"user"} = $user;
		$pid_bucket{"ppid"} = $parent_pid;
		$pid_bucket{"listens"} = \@listens;
		$pid_bucket{"targets"} = \@targets;
    if (!%{$mytraces{$host}}) {
				print "THERE\n";
				my %host_bucket;
				%{$host_bucket{$pid}} = %pid_bucket;
				%{$mytraces{$host}} = %host_bucket;
		} else {
				print "HERE\n";
				%{$mytraces{$host}{$pid}} = %pid_bucket;
		}
		
		if (!$mytraces{$host}{$parent_pid}) {
				my %pid_bucket;
				$pid_bucket{"exe_name"} = $parent_exe_name;
				$pid_bucket{"user"} = $parent_user;
				%{$mytraces{$host}{$parent_pid}} = %pid_bucket;
		}
}

# Match communication target node
sub match_node {
		my $host = shift @_;
		my $port = shift @_;

		my %host_bucket = %{$mytraces{$host}};
		foreach $pid (keys %host_bucket) {
				my %pid_bucket = %{$host_bucket{$pid}};
				my @listens = @{$pid_bucket{"listens"}};
				foreach $l (@listens) {
						if ($l eq $port) {
								return $pid;
						}
				}
		}
		return "";
}

# Draw edges
sub draw_graph {
		my %shapes;
		$shapes{"192.168.8.133"}="circle";
		$shapes{"192.168.8.134"}="star";
		$shapes{"192.168.8.135"}="triangle";

		# Draw communication edges
		my %uniq_edges;
		foreach $host (keys %mytraces) {
				my %host_bucket = %{$mytraces{$host}};
				foreach $pid (keys %host_bucket) {
						my %pid_bucket = %{$host_bucket{$pid}};
						my @targets = @{$pid_bucket{"targets"}};
						foreach $h (@targets) {
								my ($trg_host, $trg_port) = split(":", $h);
								print "connect to $trg_host:$trg_port\n";
								my $trg_pid = &match_node($trg_host, $trg_port);
								if (length($trg_pid) > 0) {
										my $node_id = "pid=$pid\@$host";
										my $trg_node_id = "pid=$trg_pid\@$trg_host";
										my $uid="$node_id==$trg_node_id";
										$uniq_edges{$uid} = 1;
										print "$node_id => $trg_node_id\n";
								}
						}
				}
		}

		# Draw nodes
		print "===== Draw graph =====\n";
		foreach $host (keys %mytraces) {
				# Draw clusters by host

				my %host_bucket = %{$mytraces{$host}};
				foreach $pid (keys %host_bucket) {
						print "$pid\n";

						# Draw processes in cluster
						my $node_id = "pid=$pid\@$host";
						my $exe_name = $host_bucket{$pid}{"exe_name"};
						my $user = $host_bucket{$pid}{"user"};

						$json .= "{\"adjacencies\": [\n";

						foreach $uid (keys %uniq_edges) {
								my ($nodeid, $trg_node_id) = split("==", $uid);
								if ($nodeid eq $node_id) {
										$json .= "{\n";
										$json .= "\"nodeTo\": \"$trg_node_id\",\n";
										$json .= "\"nodeFrom\": \"$nodeid\",\n";
#										$json .= "\"data\": {\"\$color\":\"#BCBDDC\"}\n";
										$json .= "\"data\": {}\n";
										$json .= "},\n";
								}
						}

						my $ppid = $host_bucket{$pid}{"ppid"};
						if ($ppid) {
								my $nodeid = "pid=$ppid\@$host";
								my $trg_node_id = "pid=$pid\@$host";
								$json .= "{\n";
								$json .= "\"nodeFrom\": \"$trg_node_id\",\n";
								$json .= "\"nodeTo\": \"$nodeid\",\n";
								$json .= "\"data\": {\"\$color\":\"#FFEDA0\"}\n";
								$json .= "},\n";
						}

						$color = $colors{$user};
						if ($color eq "") {
								$color = "#BDBDBD";
						}
						$json .= "], \n";
						$json .= "\"data\": {\n";
						$json .= "\"\$color\": \"$color\",\n";
						$json .= "\"\$type\": \"$shapes{$host}\"\n";
						$json .= "},\n";
						$json .= "\"id\": \"$node_id\",\n";
						$json .= "\"name\": \"$exe_name\"\n";
						$json .= "},\n";
				}

				# Draw parent-child edges
				foreach $pid (keys %host_bucket) {
						my $ppid = $host_bucket{$pid}{"ppid"};
						if ($ppid) {
								my $parent_node_id = "pid=$ppid\@$host";
								my $child_node_id = "pid=$pid\@$host";
						}
				}
		}

}

#------------------------------------------------------------
# Main
#------------------------------------------------------------
opendir DIR, $xml_dir or die $!;
my @dirs = readdir(DIR);
foreach $d (@dirs) {
		if ($d=~m/.+host/) {
				opendir DIR2, "$xml_dir/$d" or die $!;
				my @files = readdir(DIR2);
				foreach $f (@files) {
						if ($f=~ m/.+\.xml/) {
								&parse_xml("$xml_dir/$d/$f");
						}
				}
				close DIR2;
		}
}
closedir DIR;

$json .= "var json = [\n";
&draw_graph();
$json .= "];\n";

print "****\n";
print $json, "\n";
