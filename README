INFOS
==============================================================================

InfOS is the Informatics research operating system, designed specifically for
the UG3 Operating Systems course.

InfOS has been designed by Tom Spink <tspink@inf.ed.ac.uk> to be used as a
teaching aid, and forms the basis of the OS coursework.  If you have any
questions/problems, please get in touch with Tom in the first instance.

The operating system has been designed from scratch, from the ground-up,
and tries to use modern C++ programming paradigms.  As it's an ongoing work
in progress, there are plenty of places that need improvement.

RUNNING
==============================================================================

To try out InfOS, you can use QEMU in x86-64 system mode.  To do anything
useful, you'll also need the user-space, which you can check-out from:

https://github.com/tspink/infos-user

Here are some commands to get you started:

# git clone https://github.com/tspink/infos
# git clone https://github.com/tspink/infos-user
# cd infos
# make
# cd ../infos-user
# make
# make fs
# qemu-system-x86_64 -m 8G \
  -kernel ../infos/out/infos-kernel \
  -debugcon stdio \
  -hda bin/rootfs.tar \
  -append 'pgalloc.debug=0 pgalloc.algorithm=simple objalloc.debug=0 sched.debug=0 sched.algorithm=cfs syslog=serial boot-device=ata0 init=/usr/init'

This should boot InfOS in QEMU, starting the example user-space.

Since this project was created for a course at the University of Edinburgh,
it is /moderately/ bespoke, although it is technically a general purpose
operating system.  If you are interested in the coursework, get in touch
and I can supply the documentation!

tspink@inf.ed.ac.uk
