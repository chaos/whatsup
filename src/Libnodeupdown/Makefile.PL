use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

# achu: MakeMaker is quite limited, and seems to have some screwy
# things.  This hacked up thing is the best I can do right now.  I'm
# sure there's a better way.

WriteMakefile(
    'NAME'		=> 'Libnodeupdown',
    'VERSION_FROM'      => 'Libnodeupdown.pm',          
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Libnodeupdown.pm', # retrieve abstract from module
       AUTHOR     => 'Albert Chu <chu11@llnl.gov>') : ()),
    # Will not link any libraries that can't be found
    'LIBS'		=> ['-lgenders -lgendersllnl'],
    # HAVE_CONFIG_H for config.h
    # Undefine VERSION, it collides with VERSION from config.h
    'DEFINE'		=> '-DHAVE_CONFIG_H -UVERSION',
     
    # If you do hostlist.o nodeupdown.o ... etc.  MakeMaker fails.  I
    # don't know why.  I've made it the responsibility of Makefile.am
    # to copy only the C files we must use.

    'OBJECT'		=> '$(O_FILES)',
);
