#!/usr/bin/perl -w

use Nodeupdown;

$obj = Nodeupdown->new();

$ret = $obj->up_nodes();
print "$ret\n\n";

$ret = $obj->down_nodes();
print "$ret\n\n";

@list = $obj->get_up_nodes();
foreach $i (@list) {
    print "$i ";
}
print "\n\n";

@list = $obj->down_nodes();
foreach $i (@list) {
    print "$i ";
}
print "\n\n";

$obj->up_nodes();
$obj->down_nodes();

print $obj->are_up("mdevi"),"\n";
print $obj->are_up("emdevi"),"\n";
print $obj->are_up("foomdevi"),"\n";
print $obj->are_up("mdevi","mdevj"),"\n";
print $obj->are_up("emdevi","mdevj"),"\n";
print $obj->are_up("foomdevi","mdevj"),"\n";
print $obj->are_up();

print $obj->are_down("mdevi"),"\n";
print $obj->are_down("emdevi"),"\n";
print $obj->are_down("foomdevi"),"\n";
print $obj->are_down("mdevi","mdevj"),"\n";
print $obj->are_down("emdevi","mdevj"),"\n";
print $obj->are_down("foomdevi","mdevj"),"\n";
print $obj->are_down();
