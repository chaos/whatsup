#!/usr/bin/perl -w

use Nodeupdown;

$obj = Nodeupdown->new();

$ret = $obj->up_nodes();
print "$ret\n\n";

$ret = $obj->down_nodes();
print "$ret\n\n";

@list = $obj->up_nodes();
foreach $i (@list) {
    print "$i ";
}
print "\n\n";

@list = $obj->down_nodes();
foreach $i (@list) {
    print "$i ";
}
print "\n\n";

print $obj->are_up("devi"),"\n";
print $obj->are_up("devi","dev0"),"\n";

print $obj->are_down("devi"),"\n";
print $obj->are_down("edevi"),"\n";
