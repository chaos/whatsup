use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

# achu: MakeMaker seems quite limited. This hacked up thing is
# the best I can do right now.  I'm sure there's a better way.

WriteMakefile(
    'NAME'		=> 'Libnodeupdown',
    'VERSION_FROM'      => 'Libnodeupdown.pm',          
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Libnodeupdown.pm', # retrieve abstract from module
       AUTHOR     => 'Albert Chu <chu11@llnl.gov>') : ()),
    # Will not link any libraries that can't be found
    'LIBS'		=> ['-lgenders -lgendersllnl'],
    # Undefine VERSION, it collides with VERSION from config.h
    'DEFINE'		=> '-DHAVE_CONFIG_H -UVERSION', # for config.h
    'INC'		=> '-I../../config -I../libcommon -I../libdotconf -I../libexpat -I../libnodeupdown',  # for config.h
    'OBJECT'		=> 'fd.o list.o hostlist.o dotconf.o readdir.o xmlparse.o xmlrole.o xmltok.o nodeupdown.o nodeupdown_masterlist.o',
);
