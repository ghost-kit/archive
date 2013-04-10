#-*- mode: makefile -*-

include ../../../env/Make.${MACHINE}
include Make.buildenv
#include ../Make.$(shell uname)

.DEFAULT_GOAL := all

CXXFLAGS += $(OPTLVL)
CXXFLAGS += -w

PROGS = test benchmark

LIBS = libIo.a libIoApp.a

all: libIo.a libIoApp.a libIoNopp.a libIoNoppNompi.a

OBJS = Io.o Hdf.o Hdf4.o Hdf5.o PHdf5.o

NULLMPI = null-mpi/null-mpi.o

#$(OBJS) $(patsubst %.o,%-App.o,$(OBJS)): $(wildcard *.hpp)

libIo.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $+

libIoApp.a: $(patsubst %.o,%-App.o,$(OBJS) $(NULLMPI))
	$(AR) $(ARFLAGS) $@ $+

libIoNopp.a: $(patsubst %.o,%-Nopp.o,$(OBJS))
	$(AR) $(ARFLAGS) $@ $+

libIoNoppNompi.a: $(patsubst %.o,%-NoppNompi.o,$(OBJS) $(NULLMPI))
	$(AR) $(ARFLAGS) $@ $+

unitTest:
	${MAKE} -f Make.test

distclean: clean

clean:
	rm -f $(PROGS) $(patsubst %,%-App,$(PROGS)) *.a *.o null-mpi/*.o *.hdf *.hdf4 *.hdf5 *.phdf5
	${MAKE} -f Make.test clean

$(OBJS) $(patsubst %,%.o,$(PROGS)): %.o : %.C
	$(MPICXX) -c $(CXXFLAGS) $(IOFLAGS) $(IOINCS) $^ -o $@

$(patsubst %.o,%-App.o,$(OBJS) $(NULLMPI)) $(patsubst %,%-App.o,$(PROGS)): %-App.o : %.C
	$(CXX) -c $(CXXFLAGS) $(IOAPPFLAGS) $(IOAPPINCS) $^ -o $@

$(patsubst %.o,%-Nopp.o,$(OBJS)) $(patsubst %,%-Nopp.o,$(PROGS)): %-Nopp.o : %.C
	$(MPICXX) -c $(CXXFLAGS) $(IONOPPFLAGS) $(IONOPPINCS) $^ -o $@

$(patsubst %.o,%-NoppNompi.o,$(OBJS) $(NULLMPI)) $(patsubst %,%-NoppNompi.o,$(PROGS)): %-NoppNompi.o : %.C
	$(CXX) -c $(CXXFLAGS) $(IONOPPNOMPIFLAGS) $(IONOPPNOMPIINCS) $^ -o $@

$(PROGS): % : %.o libIo.a
	$(MPICXX) -g -o $@ $+ $(LDFLAGS) $(IOLIBS) 

$(patsubst %,%-App,$(PROGS)): % : %.o libIoApp.a
	$(CXX) -g -o $@ $+ $(LDFLAGS) $(IOAPPLIBS)

$(patsubst %,%-Nopp,$(PROGS)): % : %.o libIoNopp.a
	$(MPICXX) -g -o $@ $+ $(LDFLAGS) $(IONOPPLIBS)

$(patsubst %,%-NoppNompi,$(PROGS)): % : %.o libIoNoppNompi.a
	$(CXX) -g -o $@ $+ $(LDFLAGS) $(IONOPPNOMPILIBS)
