use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Libnodeupdown',
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Libnodeupdown.pm', # retrieve abstract from module
       AUTHOR     => 'A. U. Thor <a.u.thor@a.galaxy.far.far.away>') : ()),
    'LIBS'		=> ['-lgenders -lgendersllnl -lganglia'], # e.g., '-lm'
    # Undefine VERSION, it collides with VERSION from config.h
    'DEFINE'		=> '-DHAVE_CONFIG_H -UVERSION', # for config.h
    'INC'		=> '-I../../config',  # for config.h
	# Un-comment this if you add C files to link with later:
    'OBJECT'		=> '$(O_FILES)', # link all the C files too
);
