# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
BEGIN { plan tests => 1 };
use Libnodeupdown;

ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

$handle = Libnodeupdown->nodeupdown_handle_create();
if (!defined($handle)) {
    print "Error, nodeupdown_handle_create()\n";
    exit(1);
}

$ret = $handle->nodeupdown_load_data();
if ($ret == -1) {
    print "Error, nodeupdown_load_data()\n";
    print $handle->nodeupdown_errormsg();
    exit(1);
}

$ret = $handle->nodeupdown_get_up_nodes_string();
print "$ret\n\n";

$ret = $handle->nodeupdown_get_down_nodes_string();
print "$ret\n\n";

$ret = $handle->nodeupdown_get_up_nodes_list();
foreach $i (@$ret) {
    print "$i ";
}
print "\n\n";

$ret = $handle->nodeupdown_get_down_nodes_list();
foreach $i (@$ret) {
    print "$i ";
}
print "\n\n";

print $handle->nodeupdown_is_node_up("mdevi"),"\n";
print $handle->nodeupdown_is_node_up("emdevi"),"\n";
print $handle->nodeupdown_is_node_up("foomdevi"),"\n";
print $handle->nodeupdown_is_node_down("mdevi"),"\n";
print $handle->nodeupdown_is_node_down("emdevi"),"\n";
print $handle->nodeupdown_is_node_down("foomdevi"),"\n";
