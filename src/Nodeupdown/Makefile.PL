use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Nodeupdown',
    'VERSION_FROM'	=> 'Nodeupdown.pm',
    'PREREQ_PM'		=> {},
    ($] >= 5.005 ?
      (ABSTRACT_FROM => 'Nodeupdown.pm',
       AUTHOR     => 'Albert Chu <chu11@llnl.gov>') : ()),
);
