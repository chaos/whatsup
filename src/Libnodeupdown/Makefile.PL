use ExtUtils::MakeMaker;

WriteMakefile(
    'NAME'		=> 'Libnodeupdown',
    'VERSION_FROM'      => 'Libnodeupdown.pm',          
    'PREREQ_PM'		=> {},
    ($] >= 5.005 ?
      (ABSTRACT_FROM => 'Libnodeupdown.pm',
       AUTHOR     => 'Albert Chu <chu11@llnl.gov>') : ()),
);
