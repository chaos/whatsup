use ExtUtils::MakeMaker;

WriteMakefile(
    'NAME'		=> 'Libnodeupdown',
    'VERSION_FROM'      => 'Libnodeupdown.pm',          
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Libnodeupdown.pm', # retrieve abstract from module
       AUTHOR     => 'Albert Chu <chu11@llnl.gov>') : ()),
);
