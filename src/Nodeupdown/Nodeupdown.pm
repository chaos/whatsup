;#############################################################################
# $Id: Nodeupdown.pm,v 1.1 2003-08-18 16:28:01 achu Exp $
# $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/Nodeupdown/Nodeupdown.pm,v $
#############################################################################

package Nodeupdown;

use strict;
use Libnodeupdown;

our $VERSION = "2.0";

require Exporter;
our @ISA = qw(Exporter);

our @EXPORT_OK = qw(&_errormsg 
                    $debugkey 
                    $handlekey
                    $hostkey
                    $portkey,
                    $NODEUPDOWN_TIMEOUT_LEN
                    $NODEUPDOWN_CONF_FILE);
our %EXPORT_TAGS = ( 'all' => [ qw(&_errormsg 
                                   $debugkey 
                                   $handlekey
                                   $hostkey
                                   $portkey
                                   $NODEUPDOWN_TIMEOUT_LEN
                                   $NODEUPDOWN_CONF_FILE) ] );

our $NODEUPDOWN_TIMEOUT_LEN = Libnodeupdown->NODEUPDOWN_TIMEOUT_LEN
our $NODEUPDOWN_CONF_FILE = Libnodeupdown->NODEUPDOWN_CONF_FILE
our $debugkey = "_DEBUG";
our $handlekey = "_HANDLE";
our $hostkey = "_HOST";
our $portkey = "_PORT";

sub _errormsg {
    my $self = shift;
    my $msg = shift;
    my $str;

    if ($self->{$debugkey}) {
        $str = $self->{$handlekey}->nodeupdown_errormsg();
        print STDERR "Error: $msg, $str\n";
    }
}

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $hostname = shift;
    my $port = shift;
    my $self = {};
    
    if (defined($hostname)) {
        $self->{$hostkey} = $hostname;
    }
    else {
        $self->{$hostkey} = undef;
    }

    if (defined($port)) {
        $self->{$portkey} = $port;
    }
    else {
        $self->{$portkey} = undef;
    }

    $self->{$debugkey} = 0;
    
    $handle = Libnodeupdown->nodeupdown_handle_create();
    if (!defined($handle)) {
        _errormsg($self, "nodeupdown_handle_create()");
        return undef;
    }

    $self->{$handlekey} = $handle;

    $ret = $self->{$handlekey}->nodeupdown_load_data(undef,
                                                     $self->{$hostkey},
                                                     undef,
                                                     $self->{$portkey},
                                                     undef);
    if ($ret == -1) {
        _errormsg($self, "nodeupdown_load_data()");
        return undef;
    } 

    bless ($self, $class);
    return $self;
}

sub debug {
    my $self = shift;
    my $num = shift;

    if (ref($self)) {
        if (defined $num) {
            $self->{$debugkey} = $num;
        }
    }
}

sub up_nodes {
    my $self = shift;
    my $retval;

    if (ref($self)) {

        if (!wantarray) {
            $retval = $self->{$handlekey}->nodeupdown_get_up_nodes_string();
            if (!defined($retval)) {
                _errormsg($self,"nodeupdown_get_up_nodes_string");
                return "";
            }
            return $retval;
        }
        else {
            $retval = $self->{$handlekey}->nodeupdown_get_up_nodes_list();
            if (!defined($retval)) {
                _errormsg($self,"nodeupdown_get_up_nodes_list");
                return ();
            }
            return @$retval;
        }
    }
    else {
        if (!wantarray) {
            return "";
        }
        else {
            return ();
        }
    }
}

sub down_nodes {
    my $self = shift;
    my $retval;

    if (ref($self)) {

        if (!wantarray) {
            $retval = $self->{$handlekey}->nodeupdown_get_down_nodes_string();
            if (!defined($retval)) {
                _errormsg($self,"nodeupdown_get_down_nodes_string");
                return "";
            }
            return $retval;
        }
        else {
            $retval = $self->{$handlekey}->nodeupdown_get_down_nodes_list();
            if (!defined($retval)) {
                _errormsg($self,"nodeupdown_get_down_nodes_list");
                return ();
            }
            return @$retval;
        }
    }
    else {
        if (!wantarray) {
            return "";
        }
        else {
            return ();
        }
    }
}

sub _common_check {
    my $up_or_down = shift;
    my $self = shift;
    my @list = @_;
    my $found;
    my $testnode;
    my $retval = 1;

    if (defined($self)) {

        if (@list == 0) {
            return 0;
        }

        foreach $testnode (@list) {
            $found = 0;
            if ($up_or_down == 1) {
                $found = $self->{$handlekey}->nodeupdown_is_node_up($testnode);
            }
            else {
                 $found = $self->{$handlekey}->nodeupdown_is_node_down($testnode);
            }
            
            if ($found == -1) {
                if ($up_or_down == 1) {
                    _errormsg($self,"nodeupdown_is_node_up");
                }
                else {
                    _errormsg($self,"nodeupdown_is_node_down");
                }
                return 0;
            }
            if ($found == 0) {
                $retval = 0;
                last;
            }
        }
        return $retval;
    }
    else {
        return 0;
    }
}

sub are_up {
    my @args;
    @args = (1, @_);
    return _common_check(@args);
}

sub are_down {
    my @args;
    @args = (0, @_);
    return _common_check(@args);
}

1;

__END__


=head1 NAME

Nodeupdown - Perl API for determining up and down nodes

=head1 SYNOPSIS

 use Nodeupdown;

 $obj = Nodeupdown->new([$host, [$port]])

 $upnodes = $obj->up_nodes()
 @uplist  = $obj->up_nodes()

 $downnodes = $obj->down_nodes()
 @downlist  = $obj->down_nodes()

 $bool = $obj->are_up(@nodes)
 $bool = $obj->are_down(@nodes)

=head1 DESCRIPTION

This package provides a Perl API for determining up and down nodes

=over 4

=item B<Nodeupdown-E<gt>new([$host, [$port]])>

Creates and returns a Nodeupdown object.  If the host or port are not
specified, default values are assumed.  On error, undef is returned.

=item B<$obj-E<gt>up_nodes()>

Return the up nodes in a cluster.  If the context in which the 
subroutine is invoked is looking for a scalar, a hostlist formatted
string of up nodes will be returned.  If the context in which the 
subroutine is invoked is looking for a list, a list containing
each up node will be returned.

=item B<$obj-E<gt>down_nodes()>

Return the down nodes in a cluster.  If the context in which the 
subroutine is invoked is looking for a scalar, a hostlist formatted
string of down nodes will be returned.  If the context in which the 
subroutine is invoked is looking for a list, a list containing
each down node will be returned.

=item B<$obj-E<gt>are_up(@nodes)>

Returns 1 is all the nodes passed in are determined as up.  Returns 0
if any node passed is not up.

=item B<$obj-E<gt>are_down(@nodes)>

Returns 1 is all the nodes passed in are determined as down.  Returns 0
if any node passed is not down.

=back 

=head1 BUGS

Please be careful with the semantics of B<are_up()> and B<are_down()>.
Just because a node is not up, does not mean it is down.  For example,
if an improper node name is used (i.e. $obj->are_up("foobar")), both
B<are_up()> and B<are_down()> will fail.

=head1 AUTHOR

Albert Chu E<lt>chu11@llnl.govE<gt>

=head1 SEE ALSO

L<libnodeupdown>.

L<whatsup>.